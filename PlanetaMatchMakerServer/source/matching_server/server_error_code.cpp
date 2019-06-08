#include "server_error_code.hpp"

namespace pgl {
	std::string get_server_error_message(const server_error_code error_code) {
		switch (error_code) {
		case server_error_code::ok:
			return "Ok.";
		case server_error_code::acception_failed:
			return "Failed to accept new client.";
		case server_error_code::message_reception_timeout:
			return "Failed to receive message because of time out.";
		case server_error_code::message_header_reception_error:
			return "Failed to receive message header.";
		case server_error_code::message_body_reception_error:
			return "Failed to receive message body.";
		case server_error_code::invalid_message_type:
			return "Message type is invalid.";
		case server_error_code::message_type_mismatch:
			return "Message type does not match to expected one.";
		default:
			return "Unknown";
		}
	}
}
