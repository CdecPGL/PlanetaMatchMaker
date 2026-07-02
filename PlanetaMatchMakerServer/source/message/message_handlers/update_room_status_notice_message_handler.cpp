#include <optional>

#include "update_room_status_notice_message_handler.hpp"
#include "session/session_data.hpp"
#include "../message_parameter_validator.hpp"

using namespace minimal_serializer;

namespace pgl {
	update_room_status_notice_message_handler::handle_return_t
	update_room_status_notice_message_handler::handle_message(const update_room_status_notice_message& message,
		const std::shared_ptr<message_handle_parameter> param) {
		const message_parameter_validator parameter_validator(param);

		// Check room group existence
		auto& room_data_container = param->server_data.get_room_data_container();

		const auto client_endpoint = endpoint::make_from_boost_endpoint(param->socket.remote_endpoint());
		const auto validate_host = [&](const room_data& target_room_data) {
			if (target_room_data.host_endpoint == client_endpoint) { return; }

			const auto error_message = generate_string("The client is not host of requested ",
				target_room_data, ". Room host endpoint is ", target_room_data.host_endpoint.to_boost_endpoint(), ".");
			throw client_error(client_error_code::room_permission_denied, false, error_message);
		};
		const auto validate_current_player_count = [&](const room_data& target_room_data) {
			if (!message.is_current_player_count_changed) { return; }
			if (message.current_player_count > target_room_data.max_player_count) {
				const auto error_message = generate_string("New player count \"",
					message.current_player_count, "\" exceeds max player count \"", target_room_data.max_player_count,
					"\" for ", target_room_data, ".");
				throw client_error(client_error_code::request_parameter_wrong, false, error_message);
			}
		};
		auto updated_room_data = std::optional<room_data>();

		// Change status of requested room
		switch (message.status) {
			case update_room_status_notice_message::status::open:
				updated_room_data = room_data_container.try_update_with_host_reported_current_player_count(message.room_id,
					message.is_current_player_count_changed, message.current_player_count,
					[&](auto& target_room_data) {
						validate_host(target_room_data);
						validate_current_player_count(target_room_data);
						target_room_data.setting_flags |= room_setting_flag::open_room;
					});
				if (!updated_room_data.has_value()) { parameter_validator.throw_room_not_found_error(message.room_id); }
				log_with_endpoint(log_level::info, param->socket.remote_endpoint(), "Open ", *updated_room_data, ".");
				break;
			case update_room_status_notice_message::status::close:
				updated_room_data = room_data_container.try_update_with_host_reported_current_player_count(message.room_id,
					message.is_current_player_count_changed, message.current_player_count,
					[&](auto& target_room_data) {
						validate_host(target_room_data);
						validate_current_player_count(target_room_data);
						target_room_data.setting_flags &= ~room_setting_flag::open_room;
					});
				if (!updated_room_data.has_value()) { parameter_validator.throw_room_not_found_error(message.room_id); }
				log_with_endpoint(log_level::info, param->socket.remote_endpoint(), "Close ", *updated_room_data, ".");
				break;
			case update_room_status_notice_message::status::remove:
				updated_room_data = room_data_container.try_remove_if(message.room_id, [&](const auto& target_room_data) {
					validate_host(target_room_data);
					validate_current_player_count(target_room_data);
					return true;
				});
				if (!updated_room_data.has_value()) { parameter_validator.throw_room_not_found_error(message.room_id); }
				log_with_endpoint(log_level::info, param->socket.remote_endpoint(), "Remove ", *updated_room_data, ".");
				param->session_data.delete_hosting_room_id(updated_room_data->room_id);
				break;
			default:
				const auto error_message = generate_string("The new status \"", message.status, "\" for room \"",
					message.room_id, "\" is invalid.");
				throw client_error(client_error_code::request_parameter_wrong, false, error_message);
		}

		return {{}, false};
	}
}
