#pragma once

#include <algorithm>
#include <chrono>
#include <memory>
#include <span>
#include <utility>
#include <vector>

#include "async/timer.hpp"
#include "async/read_write.hpp"
#include "server/server_errors.hpp"
#include "server/server_data.hpp"
#include "logger/log.hpp"
#include "messages.hpp"
#include "message_handle_parameter.hpp"
#include "session/session_data.hpp"

namespace pgl {
	template <typename ... Params>
	void log_with_session(const log_level level, const message_handle_parameter& param, Params&& ... params) {
		if (const auto session_number = param.session_data.session_number(); session_number.has_value()) {
			log_with_session_and_endpoint(level, *session_number, param.session_data.remote_endpoint().to_boost_endpoint(),
				std::forward<Params>(params)...);
		}
		else {
			log_with_endpoint(level, param.session_data.remote_endpoint().to_boost_endpoint(),
				std::forward<Params>(params)...);
		}
	}

	template <typename ... Params>
	void log_with_session(const log_level level, const std::shared_ptr<message_handle_parameter>& param,
		Params&& ... params) {
		log_with_session(level, *param, std::forward<Params>(params)...);
	}

	// Send data to remote endpoint. server_session_error will be thrown when send error occurred.
	template <serializable FirstData, serializable... RestData>
	void send(std::shared_ptr<message_handle_parameter> param, FirstData&& first_data, RestData&&... rest_data) {
		auto data_summary = minimal_serializer::generate_string(sizeof...(rest_data) + 1, " data (",
			get_packed_size<FirstData, RestData...>(), " bytes)");

		try {
			execute_socket_timed_async_operation(param->connection, param->timeout_seconds,
				[param, first_data, rest_data...]() {
					packed_async_write(
						param->connection, param->yield, first_data, rest_data...);
				});
			log_with_session(log_level::debug, param, "Send ", data_summary,
				" to the client.");
		}
		catch (const boost::system::system_error& e) {
			auto extra_message = minimal_serializer::generate_string("Failed to send ", data_summary,
				" to the client. ",
				e.code().message());
			if (e.code() == boost::asio::error::operation_aborted) {
				throw server_session_error(extra_message + "(Failed to send message due to timeout)");
			}
			if (e.code() == boost::asio::error::eof) {
				throw server_session_error(extra_message + "(Disconnected unexpectedly)");
			}
			throw server_session_error(extra_message);
		}
	}

	// Receive data. server_session_error will be thrown when reception error occurred.
	// todo: use shared_ptr to avoid invalid reference access in lambda function
	template <typename Duration, typename FirstData, typename... RestData> requires(
		serializable_all<FirstData, RestData...> &&
		not_constant_all<FirstData, RestData...>)
	void receive_with_timeout(std::shared_ptr<message_handle_parameter> param, const Duration& timeout,
		FirstData& first_data, RestData&... rest_data) {
		auto data_summary = minimal_serializer::generate_string(sizeof...(rest_data) + 1, " data (",
			get_packed_size<FirstData, RestData...>(), " bytes)");

		try {
			execute_socket_timed_async_operation(param->connection, timeout,
				[param, &first_data, &rest_data...]()mutable {
					unpacked_async_read(param->connection, param->yield, first_data, rest_data...);
				});
			log_with_session(log_level::debug, param, "Receive ", data_summary,
				" from the client.");
		}
		catch (const boost::system::system_error& e) {
			auto extra_message = minimal_serializer::generate_string("Failed to receive ", data_summary,
				" from the client. ",
				e.code().message());
			if (e.code() == boost::asio::error::operation_aborted) {
				throw server_session_error(extra_message + "(Failed to receive message due to timeout)");
			}
			if (e.code() == boost::asio::error::eof) {
				throw server_session_error(extra_message + "(Disconnected unexpectedly)");
			}
			throw server_session_error(extra_message);
		}
	}

	template <typename FirstData, typename... RestData> requires(serializable_all<FirstData, RestData...> &&
		not_constant_all<FirstData, RestData...>)
	void receive(std::shared_ptr<message_handle_parameter> param, FirstData& first_data, RestData&... rest_data) {
		receive_with_timeout(param, param->timeout_seconds, first_data, rest_data...);
	}

	enum class message_attachment_requirement {
		forbidden,
		optional,
		required
	};

	struct message_attachment_policy final {
		message_attachment_requirement requirement = message_attachment_requirement::forbidden;
		message_attachment_size_t max_size = 0;
	};

	enum class message_attachment_error {
		none,
		missing,
		unexpected,
		size_exceeded,
		format_invalid
	};

	struct message_attachment_receive_result final {
		message_attachment_error error = message_attachment_error::none;
		std::vector<uint8_t> data;

		[[nodiscard]] bool succeeded() const { return error == message_attachment_error::none; }
	};

	inline message_attachment_error validate_message_attachment_size(const message_attachment_policy policy,
		const message_attachment_size_t size) {
		if (size != 0 && policy.requirement == message_attachment_requirement::forbidden) {
			return message_attachment_error::unexpected;
		}
		if (size > message_attachment_max_bytes || size > policy.max_size) {
			return message_attachment_error::size_exceeded;
		}
		if (size == 0 && policy.requirement == message_attachment_requirement::required) {
			return message_attachment_error::missing;
		}
		return message_attachment_error::none;
	}

	inline message_attachment_receive_result receive_message_attachment(
		const std::shared_ptr<message_handle_parameter>& param,
		const message_attachment_size_t attachment_size) {
		if (attachment_size > message_attachment_max_bytes) {
			return {message_attachment_error::size_exceeded, {}};
		}

		message_attachment_receive_result result;
		result.data.reserve(attachment_size);
		const auto deadline = std::chrono::steady_clock::now() + param->timeout_seconds;
		const auto chunk_count = (attachment_size + message_attachment_chunk_data_size - 1) /
			message_attachment_chunk_data_size;
		for (auto sequence = 0u; sequence < chunk_count; ++sequence) {
			const auto now = std::chrono::steady_clock::now();
			if (now >= deadline) {
				throw server_session_error("Failed to receive message attachment due to timeout.");
			}

			message_attachment_chunk chunk{};
			receive_with_timeout(param, deadline - now, chunk);
			const auto remaining = attachment_size - result.data.size();
			const auto expected_data_size = std::min<std::size_t>(message_attachment_chunk_data_size, remaining);
			const auto padding_begin = chunk.data.begin() + expected_data_size;
			if (chunk.sequence != sequence || chunk.data_size != expected_data_size ||
				std::ranges::any_of(padding_begin, chunk.data.end(), [](const auto value) { return value != 0; })) {
				return {message_attachment_error::format_invalid, {}};
			}
			result.data.insert(result.data.end(), chunk.data.begin(), chunk.data.begin() + chunk.data_size);
		}
		return result;
	}

	inline void send_message_attachment(const std::shared_ptr<message_handle_parameter>& param,
		const std::span<const uint8_t> attachment) {
		for (std::size_t offset = 0, sequence = 0; offset < attachment.size();
			offset += message_attachment_chunk_data_size, ++sequence) {
			message_attachment_chunk chunk{};
			chunk.sequence = static_cast<uint16_t>(sequence);
			chunk.data_size = static_cast<uint8_t>(std::min<std::size_t>(
				message_attachment_chunk_data_size, attachment.size() - offset));
			std::ranges::copy_n(attachment.begin() + offset, chunk.data_size, chunk.data.begin());
			send(param, chunk);
		}
	}

	// Return if a room id exists.
	bool does_room_exist(std::shared_ptr<message_handle_parameter> param,
		const room_data_container& room_data_container, room_id_t room_id);
}
