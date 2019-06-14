#pragma once

#include "async/timer.hpp"
#include "async/read_write.hpp"
#include "server/server_error.hpp"
#include "server/server_data.hpp"
#include "utilities/log.hpp"
#include "messages.hpp"
#include "message_handle_parameter.hpp"

namespace pgl {
	// Send data to remote endpoint. server_error will be thrown when send error occured.
	template <typename FirstData, typename... RestData>
	void send(std::shared_ptr<message_handle_parameter> param, FirstData&& first_data, RestData&& ... rest_data) {
		auto data_summary = generate_string(sizeof...(rest_data) + 1, " data (",
			sizeof(first_data) + (sizeof(rest_data) + ...), " bytes");

		try {
			if constexpr (sizeof...(RestData) > 0) {
				execute_timed_async_operation(param->io_service, param->socket, param->timeout_seconds,
					[param, first_data, rest_data...]() {
						packed_async_write(
							param->socket, param->yield, first_data, rest_data...);
					});
			} else {
				execute_timed_async_operation(param->io_service, param->socket, param->timeout_seconds,
					[param, first_data]() {
						async_write(
							param->socket,
							boost::asio::buffer(&first_data, sizeof(first_data)),
							param->yield);
					});
			}
			log_with_endpoint(log_level::debug, param->socket.remote_endpoint(), "Send ", data_summary,
				" to the client.");
		} catch (const boost::system::system_error& e) {
			auto extra_message = generate_string("Failed to send ", data_summary, " to the client. ",
				e.code().message());
			throw server_error(server_error_code::message_send_error, extra_message);
		}
	}

	// Check client is registered to this server. If not registered, reply error message to client and throw server error.
	template <message_type ReplyMessageType, class ReplyMessage>
	void check_remote_endpoint_existence(std::shared_ptr<message_handle_parameter> param,
		const ReplyMessage& reply_message) {
		// Check existence
		const auto client_address = client_address::make_from_endpoint(param->socket.remote_endpoint());
		if (param->server_data->client_data_container().is_data_exist(client_address)) {
			log_with_endpoint(log_level::debug, param->socket.remote_endpoint(),
				"The client exists in the client list.");
			return;
		}

		// Send permission error to the client
		log_with_endpoint(log_level::error, param->socket.remote_endpoint(),
			"The client does not exist in the client list.");
		reply_message_header header{
			ReplyMessageType,
			message_error_code::permission_denied
		};
		send(param, header, reply_message);
		throw server_error(server_error_code::permission_error);
	}

	// Check a room group index is valid. If it is not valid, reply error message to client and throw server error.
	template <message_type ReplyMessageType, class ReplyMessage>
	void check_room_group_existence(std::shared_ptr<message_handle_parameter> param, size_t room_group_index,
		const ReplyMessage& reply_message) {
		// Check if the id is valid
		if (param->server_data->is_valid_room_group_index(room_group_index)) {
			log_with_endpoint(log_level::debug, param->socket.remote_endpoint(), "The room group index \"",
				room_group_index, "\" exists.");
			return;
		}

		// Send room group index doesn't exist error to the client
		log_with_endpoint(log_level::error, param->socket.remote_endpoint(), "The room group index \"",
			room_group_index, "\" doesn't exist. Range of valid room group index is 0 to ",
			param->server_data->room_group_count(), ".");
		const reply_message_header header{
			ReplyMessageType,
			message_error_code::room_group_index_out_of_range
		};
		send(param, header, reply_message);
		throw server_error(server_error_code::room_group_index_out_of_range);
	}

	// Check a room id exists. If it doesn't exist, reply error message to client and throw server error.
	template <message_type ReplyMessageType, class ReplyMessage>
	void check_room_existence(std::shared_ptr<message_handle_parameter> param,
		const room_data_container& room_data_container, room_id_type room_id,
		const ReplyMessage& reply_message) {
		// Check room existence
		if (room_data_container.is_data_exist(room_id)) {
			log_with_endpoint(log_level::debug, param->socket.remote_endpoint(), "The room whose id is \"", room_id,
				"\" exists.");
			return;
		}

		// Send room doesn't exist error to the client
		log_with_endpoint(log_level::error, param->socket.remote_endpoint(), "The room whose id is \"", room_id,
			"\" doesn't exist.");
		const reply_message_header header{
			ReplyMessageType,
			message_error_code::room_does_not_exist
		};
		send(param, header, reply_message);
		throw server_error(server_error_code::room_does_not_exist);
	}
}
