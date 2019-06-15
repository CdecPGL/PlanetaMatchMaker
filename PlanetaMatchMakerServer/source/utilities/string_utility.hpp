#pragma once

#include <sstream>
#include <string>
#include <type_traits>

#include "nameof.hpp"

namespace pgl {
	inline void generate_string2(std::ostringstream&) {}

	template <typename First, typename ... Rest>
	void generate_string2(std::ostringstream& oss, First&& first, Rest&& ... rest) {
		oss << std::forward<First>(first);
		generate_string2(oss, std::forward<Rest>(rest)...);
	}

	// Convert parameters to string and concatenate them to one string. This is thread safe.
	template <typename ... Params>
	std::string generate_string(Params&& ... params) {
		std::ostringstream oss;
		oss << std::boolalpha;
		generate_string2(oss, std::forward<Params>(params)...);
		return oss.str();
	}

	// Enable enum values to output by ostream
	template <typename Enum, std::enable_if_t<std::is_enum_v<Enum>, int>  = 0>
	std::ostream& operator <<(std::ostream& os, const Enum& enum_value) {
		os << nameof::nameof_enum(enum_value);
		return os;
	}


}
