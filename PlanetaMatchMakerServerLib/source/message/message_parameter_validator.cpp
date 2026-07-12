#include "message_parameter_validator.hpp"

#include "client/client_errors.hpp"
#include "logger/log.hpp"
#include "server/server_setting.hpp"

namespace pgl {
	message_parameter_validator::
	message_parameter_validator(const std::shared_ptr<message_handle_parameter>& param) : param_(param) {}

	void message_parameter_validator::validate_room_existence(const room_data_container& room_data_container,
		const room_id_t room_id, const bool is_continuable) const {
		static_cast<void>(get_existing_room(room_data_container, room_id, is_continuable));
	}

	room_data message_parameter_validator::get_existing_room(const room_data_container& room_data_container,
		const room_id_t room_id, const bool is_continuable) const {
		if (const auto room_data = room_data_container.try_get(room_id)) {
			log_with_session(log_level::debug, param_, "The room whose id is \"", room_id,
				"\" exists.");
			return *room_data;
		}

		log_with_session(log_level::error, param_, "The room whose id is \"", room_id,
			"\" doesn't exist.");
		throw_room_not_found_error(room_id, is_continuable);
	}

	void message_parameter_validator::throw_room_not_found_error(const room_id_t room_id,
		const bool is_continuable) const {
		const auto error_message = minimal_serializer::generate_string("The room with id \"", room_id,
			"\" does not exist.");
		throw client_error(client_error_code::room_not_found, !is_continuable, error_message);
	}

	void message_parameter_validator::validate_port_number(port_number_type port_number,
		const bool is_continuable) const {
		// Check port number is valid
		if (is_port_number_valid(port_number)) { return; }

		// Throw port number invalid error
		const auto error_message = minimal_serializer::generate_string("The port number \"", port_number,
			"\" is invalid.");
		throw client_error(client_error_code::request_parameter_wrong, !is_continuable, error_message);
	}

	void message_parameter_validator::validate_player_name(const player_name_t& player_name,
		const bool is_continuable) const {
		if (player_name.length() != 0) { return; }
		throw client_error(client_error_code::request_parameter_wrong, !is_continuable, "The player name is empty.");
	}

	void message_parameter_validator::validate_max_player_count(uint8_t max_player_count,
		const bool is_continuable) const {
		if (0 < max_player_count && max_player_count <= param_->server_setting.common.max_player_per_room) { return; }

		const auto error_message = minimal_serializer::generate_string(
			"max player count(", max_player_count, ") exceeds limit(",
			param_->server_setting.common.max_player_per_room, ").");
		throw client_error(client_error_code::request_parameter_wrong, !is_continuable, error_message);
	}

	const std::shared_ptr<message_handle_parameter>& message_parameter_validator::
	get_message_handle_parameter() const { return param_; }
}
