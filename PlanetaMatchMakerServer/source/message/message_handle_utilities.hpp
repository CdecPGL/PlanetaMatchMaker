#pragma once

#include "async/timer.hpp"
#include "async/read_write.hpp"
#include "server/server_errors.hpp"
#include "server/server_data.hpp"
#include "logger/log.hpp"
#include "messages.hpp"
#include "message_handle_parameter.hpp"

namespace pgl {
	// Send data to remote endpoint. server_session_error will be thrown when send error occurred.
	template <serializable FirstData, serializable... RestData>
	void send(std::shared_ptr<message_handle_parameter> param, FirstData&& first_data, RestData&&... rest_data) {
		auto data_summary = minimal_serializer::generate_string(sizeof...(rest_data) + 1, " data (",
			get_packed_size<FirstData, RestData...>(), " bytes)");

		try {
			execute_socket_timed_async_operation(param->socket, param->timeout_seconds,
				[param, first_data, rest_data...]() {
					packed_async_write(
						param->socket, param->yield, first_data, rest_data...);
				});
			log_with_endpoint(log_level::debug, param->socket.remote_endpoint(), "Send ", data_summary,
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
	template <typename FirstData, typename... RestData> requires(serializable_all<FirstData, RestData...> &&
		not_constant_all<FirstData, RestData...>)
	void receive(std::shared_ptr<message_handle_parameter> param, FirstData& first_data, RestData&... rest_data) {
		auto data_summary = minimal_serializer::generate_string(sizeof...(rest_data) + 1, " data (",
			get_packed_size<FirstData, RestData...>(), " bytes)");

		try {
			execute_socket_timed_async_operation(param->socket, param->timeout_seconds,
				[param, &first_data, &rest_data...]()mutable {
					unpacked_async_read(param->socket, param->yield, first_data, rest_data...);
				});
			log_with_endpoint(log_level::debug, param->socket.remote_endpoint(), "Receive ", data_summary,
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

	// Return if a room id exists.
	bool does_room_exist(std::shared_ptr<message_handle_parameter> param,
		const room_data_container& room_data_container, room_id_t room_id);
}
