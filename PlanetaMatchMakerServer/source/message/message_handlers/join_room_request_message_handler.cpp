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
				message_error_code::room_permission_denied
			};
			send(param, header, reply);
			return;
		}

		// Check password if private room
		if ((room_data.setting_flags & room_setting_flag::public_room) != room_setting_flag::public_room && room_data.
			password != message.password) {
			log_with_endpoint(log_level::error, param->socket.remote_endpoint(),
				"The password is wrong for requested room \"", message.room_id,
				"\" in room group \"", message.group_index, ".");
			const reply_message_header header{
				message_type::join_room_reply,
				message_error_code::room_password_wrong
			};
			send(param, header, reply);
			return;
		}

		// Check player acceptable
		if (room_data.current_player_count >= room_data.max_player_count) {
			const reply_message_header header{
				message_type::join_room_reply,
				message_error_code::room_full
			};
			send(param, header, reply);
			const auto error_message = minimal_serializer::generate_string("Requested room \"", message.room_id,
				"\" in room group \"", message.group_index, "\" is full of players (", room_data.current_player_count,
				").");
			throw server_session_error(server_session_error_code::continuable_error, error_message);
		}
		log_with_endpoint(log_level::info, param->socket.remote_endpoint(), "Requested room \"", message.room_id,
			"\" in room group \"", message.group_index, "\" accepted new player. (", room_data.current_player_count,
			").");

		// Reply to the client
		const reply_message_header header{
			message_type::join_room_reply,
			message_error_code::ok
		};
		reply.game_host_endpoint = room_data.game_host_endpoint;
		log_with_endpoint(log_level::info, param->socket.remote_endpoint(), "Reply ", message_type::join_room_request,
			" message.");
		send(param, header, reply);

		// Disconnect
		throw server_session_error(server_session_error_code::expected_disconnection);
	}
}
