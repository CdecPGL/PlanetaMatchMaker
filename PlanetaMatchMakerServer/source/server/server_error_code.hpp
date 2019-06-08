#pragma once

#include <string>

namespace pgl {
	enum class server_error_code {
		ok,
		acception_failed,
		message_reception_timeout,
		message_header_reception_error,
		message_body_reception_error,
		invalid_message_type,
		message_type_mismatch
	};

	std::string get_server_error_message(server_error_code error_code);
}
