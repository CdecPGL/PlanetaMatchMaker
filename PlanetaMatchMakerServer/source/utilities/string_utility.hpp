#pragma once

#include <sstream>
#include <string>

namespace pgl {
	inline void generate_string2(std::ostringstream&) {}

	template <typename First, typename ... Rest>
	void generate_string2(std::ostringstream& oss, First&& first, Rest&& ... rest) {
		oss << std::forward<First>(first);
		generate_string2(oss, std::forward<Rest>(rest)...);
	}

	// Convert parameters to string and concatenate them to one string
	template <typename ... Params>
	std::string generate_string(Params&& ... params) {
		std::ostringstream oss;
		generate_string2(oss, std::forward<Params>(params)...);
		return oss.str();
	}
}
