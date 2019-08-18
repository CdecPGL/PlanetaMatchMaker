#pragma once

#include <cstdint>

namespace pgl {
	enum class message_error_code : uint8_t {
		ok,
		unknown_error,
		// server api version and client api version are not same
		api_version_mismatch,
		authentication_error,
		// when in black list
		access_denied,
		room_name_duplicated,
		room_count_reaches_limit,
		room_does_not_exist,
		room_is_not_opened,
		room_password_is_wrong,
		// not authenticated or not host of room
		permission_denied,
		join_rejected,
		player_count_reaches_limit,
		room_group_index_out_of_range,
	};
}
