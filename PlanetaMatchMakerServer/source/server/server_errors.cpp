#include <utility>

#include "nameof.hpp"

#include "server_errors.hpp"

using namespace std::string_literals;

namespace pgl {
	server_error::server_error(const bool is_disconnect_required, std::string extra_message) : extra_message_(std::move(
		extra_message)), is_disconnect_required_(is_disconnect_required) {}

	bool server_error::is_disconnect_required() const { return is_disconnect_required_; }

	std::string server_error::extra_message() const { return extra_message_; }

	std::string server_error::message() const {
		auto message = "Server error."s;
		if (extra_message_.length()) { return message + " (" + extra_message_ + ")"; }
		return message;
	}

	std::ostream& operator<<(std::ostream& os, const server_error& error) {
		os << error.message();
		return os;
	}

	server_session_intended_disconnect_error::server_session_intended_disconnect_error(
		std::string extra_message) :
		extra_message_(std::move(extra_message)) {}

	std::string server_session_intended_disconnect_error::extra_message() const { return extra_message_; }

	std::string server_session_intended_disconnect_error::message() const {
		auto message = "Server error."s;
		if (extra_message_.length()) { return message + " (" + extra_message_ + ")"; }
		return message;
	}

	std::ostream& operator<<(std::ostream& os, const server_session_intended_disconnect_error& error) {
		os << error.message();
		return os;
	}

	server_session_error::server_session_error(std::string extra_message): extra_message_(std::move(extra_message)) {}

	std::string server_session_error::extra_message() const { return extra_message_; }

	std::string server_session_error::message() const {
		return "Session error. " + extra_message_;
	}

	std::ostream& operator<<(std::ostream& os, const server_session_error& error) {
		os << error.message();
		return os;
	}
}
