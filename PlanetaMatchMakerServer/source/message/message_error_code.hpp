#pragma once

#include <cstdint>

namespace pgl {
	enum class message_error_code : uint8_t {
		ok,
		unknown_error,
		version_mismatch,
		authentication_error,
		// when in black list
		denied,
		room_name_duplicated,
		room_count_reaches_limit,
		room_not_exist,
		permission_denied,
		join_rejected,
		player_count_reaches_limit
	};
}
