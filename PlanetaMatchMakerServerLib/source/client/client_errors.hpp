#pragma once

#include <string>

#include "client_error_code.hpp"

namespace pgl {
	/**
	 * @brief A class of errors due to client in message handling.
	 */
	class client_error final : public std::exception {
	public:
		client_error(client_error_code error_code, bool is_disconnect_required, std::string extra_message = "");
		[[nodiscard]] client_error_code error_code() const;
		[[nodiscard]] bool is_disconnect_required() const;
		[[nodiscard]] std::string extra_message() const;
		[[nodiscard]] std::string message() const;

	private:
		const client_error_code error_code_;
		const std::string extra_message_;
		const bool is_disconnect_required_;
	};

	std::ostream& operator <<(std::ostream& os, const client_error& error);
}
