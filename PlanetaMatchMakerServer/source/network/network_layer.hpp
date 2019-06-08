#pragma once

#include <cstdint>
#include <array>

namespace pgl {
	enum class ip_version { v4, v6 };

	using ip_address_type = std::array<uint64_t, 2>;
}
