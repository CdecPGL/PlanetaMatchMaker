#include "server_session_error.hpp"
#include <utility>

namespace pgl {
	server_session_error::server_session_error(const server_session_error_code error_code) : server_session_error(error_code, "") {}

	server_session_error::
	server_session_error(const server_session_error_code error_code, std::string extra_message): error_code_(error_code),
		extra_message_(std::move(extra_message)) {}

	server_session_error_code server_session_error::error_code() const { return error_code_; }

	std::string server_session_error::extra_message() const { return extra_message_; }

	std::string server_session_error::message() const {
		if (extra_message_.length()) { return get_server_error_message(error_code_) + "(" + extra_message_ + ")"; }
		return get_server_error_message(error_code_);
	}

	std::ostream& operator<<(std::ostream& os, const server_session_error& error) {
		os << error.message();
		return os;
	}
}
