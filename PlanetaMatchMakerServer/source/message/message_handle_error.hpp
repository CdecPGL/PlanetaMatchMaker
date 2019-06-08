#pragma once

#include <string>

#include "message_handle_error_code.hpp"

namespace pgl {
	class message_handle_error final : public std::exception {
	public:
		explicit message_handle_error(message_handle_error_code error_code);
		message_handle_error(message_handle_error_code error_code, std::string extra_message);
		[[nodiscard]] message_handle_error_code get_error_code() const;
		[[nodiscard]] std::string get_extra_message()const;
		[[nodiscard]] std::string get_message()const;
	private:
		message_handle_error_code error_code_;
		std::string extra_message_;
	};
}
