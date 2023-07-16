#pragma once

#include <cstdint>

namespace pgl {
	enum class authentication_result: uint8_t {
		success,
		api_version_mismatch,
		game_id_mismatch,
		game_version_mismatch,
	};
}
