#include "message_parameter_validator.hpp"

namespace pgl {

	message_parameter_validator::
	message_parameter_validator(const std::shared_ptr<message_handle_parameter> param) : param_(param) {}

	void message_parameter_validator::validate_room_group_existence(const room_group_index_t room_group_index) const {
		// Check if the id is valid
		if (does_room_group_exist(param_, room_group_index)) { return; }

		const auto error_message = minimal_serializer::generate_string("The room group with index \"", room_group_index,
			"\" does not exist.");
		throw server_session_error(server_session_error_code::continuable_error, error_message);
	}

	void message_parameter_validator::validate_room_existence(const room_data_container& room_data_container,
		const room_id_t room_id) const {
		// Check room existence
		if (does_room_exist(param_, room_data_container, room_id)) { return; }

		// Throw room doesn't exist error
		const auto error_message = minimal_serializer::generate_string("The room with id \"", room_id,
			"\" does not exist.");
		throw server_session_error(server_session_error_code::continuable_error, error_message);
	}

	void message_parameter_validator::validate_port_number(port_number_type port_number) const {
		// Check port number is valid
		if (is_port_number_valid(port_number)) { return; }

		// Throw port number invalid error
		const auto error_message = minimal_serializer::generate_string("The port number \"", port_number,
			"\" is invalid.");
		throw server_session_error(server_session_error_code::continuable_error, error_message);
	}

	const std::shared_ptr<message_handle_parameter>& message_parameter_validator::
	get_message_handle_parameter() const { return param_; }
}
