#include <utility>

#include "nameof.hpp"

#include "client_errors.hpp"

using namespace std::string_literals;

namespace pgl {
	client_error::client_error(const client_error_code error_code, const bool is_disconnect_required,
		std::string extra_message) : error_code_(error_code), extra_message_(std::move(extra_message)),
		is_disconnect_required_(is_disconnect_required) {}

	client_error_code client_error::error_code() const { return error_code_; }

	bool client_error::is_disconnect_required() const { return is_disconnect_required_; }

	std::string client_error::extra_message() const { return extra_message_; }

	std::string client_error::message() const {
		auto message = "Client error \""s + std::string(nameof::nameof_enum(error_code_)) + "\".";
		if (extra_message_.length()) { return message + " (" + extra_message_ + ")"; }
		return message;
	}

	std::ostream& operator<<(std::ostream& os, const client_error& error) {
		os << error.message();
		return os;
	}
}
