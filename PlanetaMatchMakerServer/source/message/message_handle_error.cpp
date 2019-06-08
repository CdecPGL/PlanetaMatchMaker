#include "message_handle_error.hpp"
#include <utility>

namespace pgl {
	message_handle_error::message_handle_error(const message_handle_error_code error_code) : error_code_(error_code) {}

	message_handle_error::
	message_handle_error(const message_handle_error_code error_code, std::string extra_message):
		error_code_(error_code), extra_message_(std::move(extra_message)) {}

	message_handle_error_code message_handle_error::get_error_code() const {
		return error_code_;
	}

	std::string message_handle_error::get_extra_message() const {
		return extra_message_;
	}

	std::string message_handle_error::get_message() const {
		if (extra_message_.length()) {
			return get_message_handle_error_message(error_code_) + "(" + extra_message_ + ")";
		}
		return get_message_handle_error_message(error_code_);
	}
}
