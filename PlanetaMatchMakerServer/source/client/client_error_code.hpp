#pragma once

#include <cstdint>
#include <iostream>

namespace pgl {
	enum class client_error_code : uint8_t {
		operation_invalid,
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
		// The number of room exceeds limit.
		room_count_exceeds_limit,
		// Connection establish mode of the room host doesn't match expected one in the client.
		room_connection_establish_mode_mismatch,
		// The client is already hosting room.
		client_already_hosting_room,
	};

	std::ostream& operator <<(std::ostream& os, const client_error_code& error_code);
}
