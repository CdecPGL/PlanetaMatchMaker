#pragma once

#include <sstream>
#include <string>
#include <type_traits>

#include "nameof.hpp"

namespace pgl {
	inline void generate_string_impl(std::ostringstream&) {}

	template <typename First, typename ... Rest>
	void generate_string_impl(std::ostringstream& oss, First&& first, Rest&& ... rest) {
		using non_cvref_first = std::remove_cv_t<std::remove_reference_t<First>>;
		// char types is recognized as character in ostringstream, so '0' means 'end of string' and '0' in char types will not displayed.
		// To avoid this, cast char types to int before pass it to ostringstream
		if constexpr (std::is_same_v<non_cvref_first, char> || std::is_same_v<non_cvref_first, unsigned char> || std::
			is_same_v<non_cvref_first, signed char>) {
			const int first_non_char = first;
			oss << first_non_char;
		} else {
			oss << std::forward<First>(first);
		}
		generate_string_impl(oss, std::forward<Rest>(rest)...);
	}

	// Convert parameters to string and concatenate them to one string. This is thread safe.
	template <typename ... Params>
	std::string generate_string(Params&& ... params) {
		std::ostringstream oss;
		oss << std::boolalpha;
		generate_string_impl(oss, std::forward<Params>(params)...);
		return oss.str();
	}

	// Enable enum values to output by ostream
	template <typename Enum, std::enable_if_t<std::is_enum_v<Enum>, int>  = 0>
	std::ostream& operator <<(std::ostream& os, const Enum& enum_value) {
		os << nameof::nameof_enum(enum_value);
		return os;
	}

	inline std::ostream& operator <<(std::ostream& os, const std::type_info& type_info) {
		os << type_info.name();
		return os;
	}
}
