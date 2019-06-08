#include "server_error.hpp"
#include <utility>

namespace pgl {
	server_error::server_error(const server_error_code error_code) : error_code_(error_code) {}

	server_error::
	server_error(const server_error_code error_code, std::string extra_message):
		error_code_(error_code), extra_message_(std::move(extra_message)) {}

	server_error_code server_error::get_error_code() const {
		return error_code_;
	}

	std::string server_error::get_extra_message() const {
		return extra_message_;
	}

	std::string server_error::get_message() const {
		if (extra_message_.length()) {
			return get_server_error_message(error_code_) + "(" + extra_message_ + ")";
		}
		return get_server_error_message(error_code_);
	}
}
