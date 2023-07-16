#include "message_parameter_validator.hpp"

#include "client/client_errors.hpp"
#include "server/server_setting.hpp"

namespace pgl {
	message_parameter_validator::
	message_parameter_validator(const std::shared_ptr<message_handle_parameter>& param) : param_(param) {}

	void message_parameter_validator::validate_room_existence(const room_data_container& room_data_container,
		const room_id_t room_id, const bool is_continuable) const {
		// Check room existence
		if (does_room_exist(param_, room_data_container, room_id)) { return; }

		// Throw room doesn't exist error
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
