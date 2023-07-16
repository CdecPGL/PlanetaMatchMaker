#pragma once

#include <cstdint>

#include "client/client_error_code.hpp"

namespace pgl {
	enum class message_error_code : uint8_t {
		ok,
		// Server internal error.
		server_error,

		//////////////////////////////////////////
		// below here is the same as client_error_code
		//////////////////////////////////////////
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

	message_error_code get_message_error_code_from_client_error_code(const client_error_code& error_code);
}
