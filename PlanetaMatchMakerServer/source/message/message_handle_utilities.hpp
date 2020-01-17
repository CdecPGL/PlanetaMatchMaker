#pragma once

#include "async/timer.hpp"
#include "async/read_write.hpp"
#include "server/server_session_error.hpp"
#include "server/server_data.hpp"
#include "logger/log.hpp"
#include "messages.hpp"
#include "message_handle_parameter.hpp"

namespace pgl {
	// Send data to remote endpoint. server_session_error will be thrown when send error occured.
	template <typename FirstData, typename... RestData>
	void send(std::shared_ptr<message_handle_parameter> param, FirstData&& first_data, RestData&& ... rest_data) {
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
				throw server_session_error(server_session_error_code::not_continuable_error,
					extra_message + "(Failed to send message due to timeout)");
			}
			if (e.code() == boost::asio::error::eof) {
				throw server_session_error(server_session_error_code::unexpected_disconnection, extra_message);
			}
			throw server_session_error(server_session_error_code::not_continuable_error, extra_message);
		}
	}

	// Receive data. server_session_error will be thrown when reception error occured.
	// todo: use shared_ptr to avoid invalid reference access in lambda function
	template <typename FirstData, typename... RestData>
	void receive(std::shared_ptr<message_handle_parameter> param, FirstData& first_data, RestData& ... rest_data) {
		static_assert(!(std::is_const_v<FirstData> || (std::is_const_v<RestData> || ...)),
			"FirstData and all RestData must not be const.");
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
				throw server_session_error(server_session_error_code::not_continuable_error,
					extra_message + "(Failed to receive message due to timeout)");
			}
			if (e.code() == boost::asio::error::eof) {
				throw server_session_error(server_session_error_code::unexpected_disconnection, extra_message);
			}
			throw server_session_error(server_session_error_code::not_continuable_error, extra_message);
		}
	}

	// Return true if a room group index is valid.
	bool does_room_group_exist(std::shared_ptr<message_handle_parameter> param, room_group_index_t room_group_index);

	// Check a room group index is valid. If it is not valid, reply error message to client and throw server error.
	template <message_type ReplyMessageType, class ReplyMessage>
	void check_room_group_existence(std::shared_ptr<message_handle_parameter> param,
		const room_group_index_t room_group_index,
		const ReplyMessage& reply_message) {
		// Check if the id is valid
		if (does_room_group_exist(param, room_group_index)) { return; }

		const reply_message_header header{
			ReplyMessageType,
			message_error_code::room_group_not_found
		};
		send(param, header, reply_message);
		const auto error_message = minimal_serializer::generate_string("The room group with index \"", room_group_index,
			"\" does not exist.");
		throw server_session_error(server_session_error_code::continuable_error, error_message);
	}

	// Check a room group index is valid. If it is not valid, throw server error.
	void check_room_group_existence(std::shared_ptr<message_handle_parameter> param,
		room_group_index_t room_group_index);

	// Return if a room id exists.
	bool does_room_exist(std::shared_ptr<message_handle_parameter> param,
		const room_data_container& room_data_container, room_id_t room_id);

	// Check a room id exists. If it doesn't exist, reply error message to client and throw server error.
	template <message_type ReplyMessageType, class ReplyMessage>
	void check_room_existence(std::shared_ptr<message_handle_parameter> param,
		const room_data_container& room_data_container, const room_id_t room_id,
		const ReplyMessage& reply_message) {
		// Check room existence
		if (does_room_exist(param, room_data_container, room_id)) { return; }

		// Send room doesn't exist error to the client
		const reply_message_header header{
			ReplyMessageType,
			message_error_code::room_not_found
		};
		send(param, header, reply_message);
		const auto error_message = minimal_serializer::generate_string("The room with id \"", room_id,
			"\" does not exist.");
		throw server_session_error(server_session_error_code::continuable_error, error_message);
	}

	// Check a room id exists. If it doesn't exist, throw server error.
	void check_room_existence(std::shared_ptr<message_handle_parameter> param,
		const room_data_container& room_data_container, room_id_t room_id);
}
