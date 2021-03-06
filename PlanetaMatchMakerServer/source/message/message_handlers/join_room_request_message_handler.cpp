#include "join_room_request_message_handler.hpp"
#include "../message_parameter_validator.hpp"

namespace pgl {
	void join_room_request_message_handler::handle_message(const join_room_request_message& message,
		std::shared_ptr<message_handle_parameter> param) {

		const message_parameter_validator_with_reply<message_type::join_room_reply, join_room_reply_message>
			parameter_validator(param);

		const auto& room_data_container = param->server_data.get_room_data_container();

		// Check room existence
		parameter_validator.validate_room_existence(room_data_container, message.room_id);
		const auto room_data = room_data_container.get_data(message.room_id);

		join_room_reply_message reply{};

		// Check if the room is open
		if ((room_data.setting_flags & room_setting_flag::open_room) != room_setting_flag::open_room) {
			log_with_endpoint(log_level::error, param->socket.remote_endpoint(), "Requested room \"", message.room_id,
				"\" is not opened.");
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
				"The password is wrong for requested room \"", message.room_id, "\".");
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
				"\" is full of players (", room_data.current_player_count,
				").");
			throw server_session_error(server_session_error_code::continuable_error, error_message);
		}
		log_with_endpoint(log_level::info, param->socket.remote_endpoint(), "Requested room \"", message.room_id,
			"\" accepted new player. (", room_data.current_player_count, ").");

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
