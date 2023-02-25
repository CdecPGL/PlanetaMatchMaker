#include "update_room_status_notice_message_handler.hpp"
#include "session/session_data.hpp"
#include "../message_parameter_validator.hpp"

namespace pgl {

	void update_room_status_notice_message_handler::handle_message(const update_room_status_notice_message& message,
		const std::shared_ptr<message_handle_parameter> param) {

		const message_parameter_validator parameter_validator(param);

		// Check room group existence
		auto& room_data_container = param->server_data.get_room_data_container();

		// Check room existence
		parameter_validator.validate_room_existence(room_data_container, message.room_id);
		auto room_data = room_data_container.get(message.room_id);

		// Check if the client is host of requested room
		if (room_data.host_endpoint != endpoint::make_from_boost_endpoint(param->socket.remote_endpoint())) {
			const auto error_message = minimal_serializer::generate_string("The client is not host of requested ",
				room_data, ". Room host endpoint is ", room_data.host_endpoint.to_boost_endpoint(), ".");
			throw server_session_error(server_session_error_code::continuable_error, error_message);
		}

		// Update current player count if need
		if (message.is_current_player_count_changed) {
			if (message.current_player_count > room_data.max_player_count) {
				const auto error_message = minimal_serializer::generate_string("New player count \"",
					message.current_player_count, "\" exceeds max player count \"", room_data.max_player_count,
					"\" for ", room_data, ".");
				throw server_session_error(server_session_error_code::continuable_error, error_message);
			}

			room_data.current_player_count = message.current_player_count;
		}

		// Change status of requested room
		switch (message.status) {
			case update_room_status_notice_message::status::open:
				log_with_endpoint(log_level::info, param->socket.remote_endpoint(), "Open ", room_data, ".");
				room_data.setting_flags |= room_setting_flag::open_room;
				room_data_container.add_or_update(room_data);
				break;
			case update_room_status_notice_message::status::close:
				log_with_endpoint(log_level::info, param->socket.remote_endpoint(), "Close ", room_data, ".");
				room_data.setting_flags &= ~room_setting_flag::open_room;
				room_data_container.add_or_update(room_data);
				break;
			case update_room_status_notice_message::status::remove:
				log_with_endpoint(log_level::info, param->socket.remote_endpoint(), "Remove ", room_data, ".");
				room_data_container.try_remove(room_data.room_id);
				param->session_data.delete_hosting_room_id(room_data.room_id);
				break;
			default:
				break;
		}
	}
}
