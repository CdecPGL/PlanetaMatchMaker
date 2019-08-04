#pragma once

#include <cstdint>

namespace pgl {
	enum class message_error_code : uint8_t {
		ok,
		unknown_error,
		// server version and client version are not same
		version_mismatch,
		authentication_error,
		// when in black list
		access_denied,
		room_name_duplicated,
		room_count_reaches_limit,
		room_does_not_exist,
		// not authenticated or not host of room
		permission_denied,
		join_rejected,
		player_count_reaches_limit,
		room_group_index_out_of_range,
	};
}
