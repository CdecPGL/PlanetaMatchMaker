#pragma once

#include <string>

#include "server_error_code.hpp"

namespace pgl {
	class server_error final : public std::exception {
	public:
		explicit server_error(server_error_code error_code);
		server_error(server_error_code error_code, std::string extra_message);
		[[nodiscard]] server_error_code error_code() const;
		[[nodiscard]] std::string extra_message()const;
		[[nodiscard]] std::string message()const;
	private:
		server_error_code error_code_;
		std::string extra_message_;
	};

	std::ostream& operator <<(std::ostream& os, const server_error& error);
}
