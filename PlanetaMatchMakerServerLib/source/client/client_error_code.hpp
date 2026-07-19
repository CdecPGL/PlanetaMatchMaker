#pragma once

#include <cstdint>
#include <iostream>

	namespace pgl {
	enum class client_error_code : uint8_t {
		operation_invalid = 0,
		// Wrong parameters which must be rejected in the client is passed for request.
		request_parameter_wrong = 1,
		// Indicated room is not found.
		room_not_found = 2,
		// Indicated password of room is not correct.
		room_password_wrong = 3,
		// The number of player reaches limit.
		room_full = 4,
		// Request is rejected because indicated room is the room which you are not host of or closed.
		room_permission_denied = 5,
		// The number of room exceeds limit.
		room_count_exceeds_limit = 6,
		// Connection establish mode of the room host doesn't match expected one in the client.
		room_connection_establish_mode_mismatch = 7,
		// The client is already hosting room.
		client_already_hosting_room = 8,
	};

	std::ostream& operator <<(std::ostream& os, const client_error_code& error_code);
}
