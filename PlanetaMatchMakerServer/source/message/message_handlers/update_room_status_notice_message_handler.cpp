#include "update_room_status_notice_message_handler.hpp"
#include "session/session_data.hpp"
#include "../message_handle_utilities.hpp"

namespace pgl {

	void update_room_status_notice_message_handler::handle_message(const update_room_status_notice_message& message,
		std::shared_ptr<message_handle_parameter> param) {

		// Check room group existence
		check_room_group_existence(param, message.group_index);
		auto& room_data_container = param->server_data.get_room_data_container(message.group_index);

		// Check room existence
		check_room_existence(param, room_data_container, message.room_id);
		auto room_data = room_data_container.get_data(message.room_id);

		// Check if the client is host of requested room
		if (room_data.host_endpoint != endpoint::make_from_boost_endpoint(param->socket.remote_endpoint())) {
			log_with_endpoint(log_level::error, param->socket.remote_endpoint(), "The client is not host of requested ",
				room_data, ".");
			throw server_error(server_error_code::room_permission_error);
		}

		// Change status of requested room
		if (message.is_current_player_count_changed) {
			room_data.current_player_count = message.current_player_count;
		}
		switch (message.status) {
			case update_room_status_notice_message::status::open:
				log_with_endpoint(log_level::info, param->socket.remote_endpoint(), "Open ", room_data, ".");
				room_data.setting_flags |= room_setting_flag::open_room;
				room_data_container.update_data(room_data);
				break;
			case update_room_status_notice_message::status::close:
				log_with_endpoint(log_level::info, param->socket.remote_endpoint(), "Close ", room_data, ".");
				room_data.setting_flags &= ~room_setting_flag::open_room;
				room_data_container.update_data(room_data);
				break;
			case update_room_status_notice_message::status::remove:
				log_with_endpoint(log_level::info, param->socket.remote_endpoint(), "Remove ", room_data, ".");
				room_data_container.remove_data(room_data.room_id);
				param->session_data.delete_hosting_room_id(message.group_index, room_data.room_id);
				break;
			default:
				break;
		}
	}
}
