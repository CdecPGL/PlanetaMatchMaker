#pragma once

#include <cstdint>

namespace pgl {
	enum class message_error_code : uint8_t {
		ok,
		// Server internal error.
		server_error,
		// Server api version doesn't match to the version the client required.
		api_version_mismatch,
		// Wrong parameters which must be rejected in the client is passed for request.
		request_parameter_wrong,
		// Indicated room is not found.
		room_not_found,
		// Indicated password of room is not correct.
		room_password_wrong,
		// The number of player reaches limit.
		room_full,
		// Request is rejected because indicated room is the room which you are not host of or closed.
		room_permission_denied,
		// Indicated room group is not found.
		room_group_not_found,
		// The number of room reaches limit.
		room_group_full,
		// Request is failed because the client is already hosting room.
		client_already_hosting_room,
	};
}
