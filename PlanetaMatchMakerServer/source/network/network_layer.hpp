#pragma once

#include <cstdint>
#include <array>

namespace pgl {
	enum class ip_version : uint8_t { v4, v6 };

	using ip_address_type = std::array<uint8_t, 16>;

	/**
	 * Convert string to ip_version. If str is invalid, throws std::out_of_range.
	 *
	 * @param str A string expression of ip address version.
	 * @return An ip address version enum.
	 * @throw std::out_of_range str is invalid.
	 */
	ip_version string_to_ip_version(const std::string& str);
}