#include "message_handle_error_code.hpp"

namespace pgl {
	std::string get_message_handle_error_message(const message_handle_error_code error_code) {
		switch (error_code) {
		case message_handle_error_code::ok:
			return "Ok.";
		case message_handle_error_code::message_reception_timeout:
			return "Failed to receive message because of time out.";
		case message_handle_error_code::message_header_reception_error:
			return "Failed to receive message header.";
		case message_handle_error_code::message_body_reception_error:
			return "Failed to receive message body.";
		case message_handle_error_code::invalid_message_type:
			return "Message type is invalid.";
		case message_handle_error_code::message_type_mismatch:
			return "Message type does not match to expected one.";
		default:
			return "Unknown";
		}
	}
}
