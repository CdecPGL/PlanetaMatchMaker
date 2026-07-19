#include <nameof.hpp>

#include "client_error_code.hpp"

namespace pgl {
	std::ostream& operator<<(std::ostream& os, const client_error_code& error_code) {
		os << std::string(nameof::nameof_enum(error_code));
		return os;
	}
}
