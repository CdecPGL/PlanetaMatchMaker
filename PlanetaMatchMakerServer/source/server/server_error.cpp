#include "server_error.hpp"
#include <utility>

namespace pgl {
	server_error::server_error(const bool is_continuable, const server_error_code error_code) : server_error(
		is_continuable, error_code, "") {}

	server_error::
	server_error(const bool is_continuable, const server_error_code error_code, std::string extra_message):
		error_code_(error_code), extra_message_(std::move(extra_message)), is_continuable_(is_continuable) {}

	server_error_code server_error::error_code() const { return error_code_; }

	std::string server_error::extra_message() const { return extra_message_; }

	std::string server_error::message() const {
		const std::string prefix = is_continuable_ ? "" : "<NOT CONTINUABLE>";
		if (extra_message_.length()) {
			return prefix + get_server_error_message(error_code_) + "(" + extra_message_ + ")";
		}
		return prefix + get_server_error_message(error_code_);
	}

	bool server_error::is_continuable() const { return is_continuable_; }

	std::ostream& operator<<(std::ostream& os, const server_error& error) {
		os << error.message();
		return os;
	}
}
