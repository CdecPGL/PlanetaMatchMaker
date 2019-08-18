#include "join_room_request_message_handler.hpp"
#include "../message_handle_utilities.hpp"

namespace pgl {
	void join_room_request_message_handler::handle_message(const join_room_request_message& message,
		std::shared_ptr<message_handle_parameter> param) {

		join_room_reply_message reply{};

		// Check room group existence
		check_room_group_existence<message_type::join_room_reply>(param, message.group_index, reply);
		const auto& room_data_container = param->server_data.get_room_data_container(message.group_index);

		// Check room existence
		check_room_existence<message_type::join_room_reply>(param, room_data_container, message.room_id, reply);
		const auto room_data = room_data_container.get_data(message.room_id);

		// Check if the room is open
		if ((room_data.setting_flags & room_setting_flag::open_room) != room_setting_flag::open_room) {
			log_with_endpoint(log_level::error, param->socket.remote_endpoint(), "Requested room \"", message.room_id,
				"\" in room group \"", message.group_index, "\" is not opened.");
			const reply_message_header header{
				message_type::join_room_reply,
				message_error_code::room_is_not_opened
			};
			send(param, header, reply);
			return;
		}

		// Check password if private room
		if ((room_data.setting_flags & room_setting_flag::public_room) != room_setting_flag::public_room && room_data.
			password != message.password) {
			log_with_endpoint(log_level::error, param->socket.remote_endpoint(), "The password is wrong for requested room \"", message.room_id,
				"\" in room group \"", message.group_index, ".");
			const reply_message_header header{
				message_type::join_room_reply,
				message_error_code::room_password_is_wrong
			};
			send(param, header, reply);
			return;
		}

		// Check player acceptable
		if (room_data.current_player_count >= room_data.max_player_count) {
			log_with_endpoint(log_level::error, param->socket.remote_endpoint(), "Requested room \"", message.room_id,
				"\" in room group \"", message.group_index, "\" is full of players (", room_data.current_player_count,
				").");
			const reply_message_header header{
				message_type::join_room_reply,
				message_error_code::player_count_reaches_limit
			};
			send(param, header, reply);
			throw server_error(server_error_code::room_player_is_full);
		}
		log_with_endpoint(log_level::info, param->socket.remote_endpoint(), "Requested room \"", message.room_id,
			"\" in room group \"", message.group_index, "\" accepted new player. (", room_data.current_player_count,
			").");

		// Reply to the client
		const reply_message_header header{
			message_type::join_room_reply,
			message_error_code::ok
		};
		reply.host_address = room_data.host_address;
		log_with_endpoint(log_level::info, param->socket.remote_endpoint(), "Reply ", message_type::join_room_request,
			" message.");
		send(param, header, reply);

		// Disconnect
		throw server_error(server_error_code::disconnected_expectedly);
	}
}
