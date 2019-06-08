#pragma once

#include <string>

namespace pgl {
	enum class message_handle_error_code {
		ok,
		message_reception_timeout,
		message_header_reception_error,
		message_body_reception_error,
		invalid_message_type,
		message_type_mismatch
	};

	std::string get_message_handle_error_message(message_handle_error_code error_code);
}
