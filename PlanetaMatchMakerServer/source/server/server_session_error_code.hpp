#pragma once

#include <string>
#include <ostream>

namespace pgl {
	enum class server_session_error_code {
		expected_disconnection,
		unexpected_disconnection,
		continuable_error,
		not_continuable_error,
	};

	std::string get_server_error_message(server_session_error_code error_code);
	std::ostream& operator <<(std::ostream& os, const server_session_error_code& error_code);
}
