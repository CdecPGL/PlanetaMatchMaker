#pragma once
#include <cstdint>

namespace pgl {
	// 4bytes
	using session_key_type = uint32_t;
	// Generate random session key
	session_key_type generate_random_session_key();
}
