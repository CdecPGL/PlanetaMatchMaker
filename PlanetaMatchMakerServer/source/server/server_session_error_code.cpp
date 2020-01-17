#include "server_session_error_code.hpp"

namespace pgl {
	std::string get_server_error_message(const server_session_error_code error_code) {
		switch (error_code) {
			case server_session_error_code::continuable_error:
				return "Continuable error. Session is continued.";
			case server_session_error_code::not_continuable_error:
				return "Not continuable error. Session will be restarted.";
			case server_session_error_code::expected_disconnection:
				return "Disconnected expectedly. This is not problem.";
			case server_session_error_code::unexpected_disconnection:
				return "Disconnected unexpectedly. Session will be restarted.";
			default:
				return "Unknown";
		}
	}

	std::ostream& operator<<(std::ostream& os, const server_session_error_code& error_code) {
		os << get_server_error_message(error_code);
		return os;
	}
}
