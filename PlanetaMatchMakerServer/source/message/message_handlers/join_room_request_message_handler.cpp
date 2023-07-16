#include "join_room_request_message_handler.hpp"
#include "../message_parameter_validator.hpp"

using namespace minimal_serializer;

namespace pgl {
	join_room_request_message_handler::handle_return_t join_room_request_message_handler::handle_message(
		const join_room_request_message& message,
		const std::shared_ptr<message_handle_parameter> param) {
		const message_parameter_validator parameter_validator(param);

		const auto& room_data_container = param->server_data.get_room_data_container();

		// Check room existence
		parameter_validator.validate_room_existence(room_data_container, message.room_id);
		const auto room_data = room_data_container.get(message.room_id);

		// Check if connection establish mode matches
		if (room_data.game_host_connection_establish_mode != message.connection_establish_mode) {
			// connection_establish_mode.others is not converted to string correctly in nameof++ so convert to string as int
			const auto error_message = generate_string("Connection establish mode of client \"",
				static_cast<uint32_t>(message.connection_establish_mode),
				"\" doesn't match one of requested room \"",
				static_cast<uint32_t>(room_data.game_host_connection_establish_mode), "\".");
			throw client_error(client_error_code::room_connection_establish_mode_mismatch, false, error_message);
		}

		// Check if the room is open
		if ((room_data.setting_flags & room_setting_flag::open_room) != room_setting_flag::open_room) {
			const auto error_message = generate_string("Requested room \"", message.room_id, "\" is not opened.");
			throw client_error(client_error_code::room_permission_denied, false, error_message);
		}

		// Check password if private room
		if ((room_data.setting_flags & room_setting_flag::public_room) != room_setting_flag::public_room && room_data.
			password != message.password) {
			const auto error_message = generate_string("The password is wrong for requested room \"", message.room_id,
				"\".");
			throw client_error(client_error_code::room_password_wrong, false, error_message);
		}

		// Check player acceptable
		if (room_data.current_player_count >= room_data.max_player_count) {
			const auto error_message = generate_string("Requested room \"", message.room_id,
				"\" is full of players (", room_data.current_player_count,
				").");
			throw client_error(client_error_code::room_full, false, error_message);
		}

		log_with_endpoint(log_level::info, param->socket.remote_endpoint(), "Requested room \"", message.room_id,
			"\" accepted new player. (", room_data.current_player_count, ").");

		// Reply to the client and Disconnect
		join_room_reply_message reply{
			room_data.game_host_endpoint,
			room_data.game_host_external_id
		};
		return {{reply}, true};
	}
}
