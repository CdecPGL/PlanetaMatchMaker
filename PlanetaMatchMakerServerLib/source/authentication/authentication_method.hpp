#pragma once

#include <cstdint>
#include <iosfwd>
#include <string>

namespace pgl {
	enum class authentication_method : uint8_t {
		none = 0,
		steam = 1,
	};

	authentication_method string_to_authentication_method(const std::string& str);
	std::string authentication_method_to_string(authentication_method method);
	std::ostream& operator <<(std::ostream& os, authentication_method method);
}
