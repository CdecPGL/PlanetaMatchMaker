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
		case server_error_code::version_mismatch:
			return "Client version does not match to server version.";
		case server_error_code::message_send_error:
			return "Failed to send message to client.";
		default:
			return "Unknown";
		}
	}

	std::ostream& operator<<(std::ostream& os, const server_error_code& error_code) {
		os << get_server_error_message(error_code);
		return os;
	}
}
