#pragma once

#include <string>

#include "server_error_code.hpp"

namespace pgl {
	class server_error final : public std::exception {
	public:
		explicit server_error(bool is_continuable, server_error_code error_code);
		server_error(bool is_continuable, server_error_code error_code, std::string extra_message);
		[[nodiscard]] server_error_code error_code() const;
		[[nodiscard]] std::string extra_message() const;
		[[nodiscard]] std::string message() const;
		[[nodiscard]] bool is_continuable() const;
	private:
		server_error_code error_code_;
		std::string extra_message_;
		bool is_continuable_;
	};

	std::ostream& operator <<(std::ostream& os, const server_error& error);
}
