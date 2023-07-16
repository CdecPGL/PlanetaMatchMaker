#include <type_traits>

#include "message_error_code.hpp"

namespace pgl {
	message_error_code get_message_error_code_from_client_error_code(const client_error_code& error_code) {
		return static_cast<message_error_code>(static_cast<std::underlying_type_t<client_error_code>>(error_code) + 2);
	}
}
