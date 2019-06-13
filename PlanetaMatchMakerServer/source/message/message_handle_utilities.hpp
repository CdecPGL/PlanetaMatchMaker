#pragma once

#include "async/timer.hpp"
#include "async/read_write.hpp"
#include "server/server_error.hpp"
#include "server/server_data.hpp"
#include "messages.hpp"
#include "message_handle_parameter.hpp"

namespace pgl {
	// Send data to remote endpoint. server_error will be thrown when send error occured.
	template <typename FirstData, typename... RestData>
	void send(std::shared_ptr<message_handle_parameter> param, FirstData&& first_data,
	          RestData&& ... rest_data) {
		try {
			if constexpr (sizeof...(RestData) > 0) {
				execute_timed_async_operation(param->io_service, param->socket, param->timeout_seconds,
				                              [param, first_data, rest_data...]()
				                              {
					                              packed_async_write(
						                              param->socket, param->yield, first_data, rest_data...);
				                              });
			} else {
				execute_timed_async_operation(param->io_service, param->socket, param->timeout_seconds,
				                              [param, first_data]()
				                              {
					                              async_write(
						                              param->socket,
						                              boost::asio::buffer(&first_data, sizeof(first_data)),
						                              param->yield);
				                              });
			}
		} catch (const boost::system::system_error& e) {
			throw server_error(server_error_code::message_send_error, e.code().message());
		}
	}

	// Check client is registered to this server. If not registered, reply error message to client and throw server error.
	template <message_type ReplyMessageType, class ReplyMessage>
	void check_remote_endpoint_authority(std::shared_ptr<message_handle_parameter> param,
	                                     const ReplyMessage& reply_message) {
		reply_message_header header{
			ReplyMessageType,
			message_error_code::permission_denied
		};

		const auto client_address = client_address::make_from_endpoint(param->socket.remote_endpoint());
		if (!param->server_data->client_data_container().is_data_exist(client_address)) {
			header.error_code = message_error_code::permission_denied;
			send(param, header, reply_message);
			throw server_error(server_error_code::permission_error);
		}
	}

	// Check a room group index is valid. If it is not valid, reply error message to client and throw server error.
	template <message_type ReplyMessageType, class ReplyMessage>
	void check_room_group_index_existence(std::shared_ptr<message_handle_parameter> param, size_t room_group_index,
	                                      const ReplyMessage& reply_message) {
		if (!param->server_data->is_valid_room_group_index(room_group_index)) {
			const reply_message_header header{
				ReplyMessageType,
				message_error_code::room_group_index_out_of_range
			};
			send(param, header, reply_message);
			const auto extra_message = generate_string("Range of valid room group index is 0 to ",
			                                           param->server_data->room_group_count(), " but \"",
			                                           room_group_index, "\" is requested.");
			throw server_error(server_error_code::room_group_index_out_of_range, extra_message);
		}
	}

	template <message_type ReplyMessageType, class ReplyMessage>
	void check_room_exists(std::shared_ptr<message_handle_parameter> param,
	                       const room_data_container& room_data_container, room_id_type room_id,
	                       const ReplyMessage& reply_message) {
		if (!room_data_container.is_data_exist(room_id)) {
			const reply_message_header header{
				ReplyMessageType,
				message_error_code::room_does_not_exist
			};
			send(param, header, reply_message);
			const auto extra_message = generate_string("Requested room name \"", room_id, "\" does not exist.");
			throw server_error(server_error_code::room_does_not_exist, extra_message);
		}
	}
}
