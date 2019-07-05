#pragma once

#include <cstdint>
#include <array>

namespace pgl {
	enum class ip_version { v4, v6 };

	using ip_address_type = std::array<uint64_t, 2>;

	// Convert string to ip_version. If str is invalid, throws std::out_of_range.
	ip_version string_to_ip_version(const std::string& str);
}
