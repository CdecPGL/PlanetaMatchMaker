#include "join_room_request_message_handler.hpp"
#include "../message_parameter_validator.hpp"

namespace pgl {
	void join_room_request_message_handler::handle_message(const join_room_request_message& message,
		const std::shared_ptr<message_handle_parameter> param) {

		const message_parameter_validator_with_reply<message_type::join_room_reply, join_room_reply_message>
			parameter_validator(param);

		const auto& room_data_container = param->server_data.get_room_data_container();

		// Check room existence
		parameter_validator.validate_room_existence(room_data_container, message.room_id);
		const auto room_data = room_data_container.get(message.room_id);

		join_room_reply_message reply{};

		// Check if connection establish mode matches
		if (room_data.game_host_connection_establish_mode != message.connection_establish_mode) {
			// connection_establish_mode.others is not converted to string correctly in nameof++ so convert to string as int
			log_with_endpoint(log_level::error, param->socket.remote_endpoint(),
				"Connection establish mode of client \"", static_cast<uint32_t>(message.connection_establish_mode),
				"\" doesn't match one of requested room \"",
				static_cast<uint32_t>(room_data.game_host_connection_establish_mode), "\".");
			constexpr reply_message_header header{
				message_type::join_room_reply,
				message_error_code::room_connection_establish_mode_mismatch
			};
			send(param, header, reply);
			return;
		}

		// Check if the room is open
		if ((room_data.setting_flags & room_setting_flag::open_room) != room_setting_flag::open_room) {
			log_with_endpoint(log_level::error, param->socket.remote_endpoint(), "Requested room \"", message.room_id,
				"\" is not opened.");
			constexpr reply_message_header header{
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
				"The password is wrong for requested room \"", message.room_id, "\".");
			constexpr reply_message_header header{
				message_type::join_room_reply,
				message_error_code::room_password_wrong
			};
			send(param, header, reply);
			return;
		}

		// Check player acceptable
		if (room_data.current_player_count >= room_data.max_player_count) {
			constexpr reply_message_header header{
				message_type::join_room_reply,
				message_error_code::room_full
			};
			send(param, header, reply);
			const auto error_message = minimal_serializer::generate_string("Requested room \"", message.room_id,
				"\" is full of players (", room_data.current_player_count,
				").");
			throw server_session_error(server_session_error_code::continuable_error, error_message);
		}
		log_with_endpoint(log_level::info, param->socket.remote_endpoint(), "Requested room \"", message.room_id,
			"\" accepted new player. (", room_data.current_player_count, ").");

		// Reply to the client
		constexpr reply_message_header header{
			message_type::join_room_reply,
			message_error_code::ok
		};
		reply.game_host_endpoint = room_data.game_host_endpoint;
		reply.game_host_external_id = room_data.game_host_external_id;
		log_with_endpoint(log_level::info, param->socket.remote_endpoint(), "Reply ", message_type::join_room_request,
			" message.");
		send(param, header, reply);

		// Disconnect
		throw server_session_error(server_session_error_code::expected_disconnection);
	}
}
