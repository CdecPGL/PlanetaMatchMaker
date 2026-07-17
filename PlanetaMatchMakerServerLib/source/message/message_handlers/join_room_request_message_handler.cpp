#include "join_room_request_message_handler.hpp"
#include "room/room_data_container.hpp"
#include "../message_parameter_validator.hpp"

using namespace minimal_serializer;

namespace pgl {
	join_room_request_message_handler::handle_return_t join_room_request_message_handler::handle_message(
		const join_room_request_message& message,
		const std::vector<uint8_t>& attachment [[maybe_unused]],
		const std::shared_ptr<message_handle_parameter> param) {
		const message_parameter_validator parameter_validator(param);

		auto& rooms = param->server_data.get_room_data_container();

		// Check room join conditions and reserve one player slot with the same locked snapshot.
		const auto join_result = rooms.try_reserve_player_for_join(message.room_id,
			message.connection_establish_mode, message.password);
		if (join_result.result == room_data_container::join_room_result::room_not_found) {
			parameter_validator.throw_room_not_found_error(message.room_id);
		}

		const auto room_data = *join_result.room;
		switch (join_result.result) {
			case room_data_container::join_room_result::connection_establish_mode_mismatch: {
				// connection_establish_mode.others is not converted to string correctly in nameof++ so convert to string as int
				const auto error_message = generate_string("Connection establish mode of client \"",
					static_cast<uint32_t>(message.connection_establish_mode),
					"\" doesn't match one of requested room \"",
					static_cast<uint32_t>(room_data.game_host_connection_establish_mode), "\".");
				throw client_error(client_error_code::room_connection_establish_mode_mismatch, false, error_message);
			}

			case room_data_container::join_room_result::room_closed: {
				const auto error_message = generate_string("Requested room \"", message.room_id, "\" is not opened.");
				throw client_error(client_error_code::room_permission_denied, false, error_message);
			}

			case room_data_container::join_room_result::password_wrong: {
				const auto error_message = generate_string("The password is wrong for requested room \"", message.room_id,
					"\".");
				throw client_error(client_error_code::room_password_wrong, false, error_message);
			}

			case room_data_container::join_room_result::room_full: {
				const auto error_message = generate_string("Requested room \"", message.room_id,
					"\" is full of players (", room_data.current_player_count,
					").");
				throw client_error(client_error_code::room_full, false, error_message);
			}

			case room_data_container::join_room_result::accepted:
				break;

			case room_data_container::join_room_result::room_not_found:
				parameter_validator.throw_room_not_found_error(message.room_id);
		}

		log_with_session(log_level::info, param, "Requested room \"", message.room_id,
			"\" accepted new player. (", room_data.current_player_count, ").");

		// Reply to the client and Disconnect
		join_room_reply_message reply{
			room_data.game_host_endpoint,
			room_data.game_host_external_id
		};
		return {
			{reply},
			true,
			[param, room_id = message.room_id] {
				param->server_data.get_room_data_container().try_release_player_join_reservation(room_id);
			}
		};
	}
}
