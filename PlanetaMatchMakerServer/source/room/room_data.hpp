#pragma once
#include <cstdint>

#include "datetime/datetime.hpp"
#include "client/client_address.hpp"

#include "room_constants.hpp"

namespace pgl {
	struct room_data final {
		room_id_type room_id;
		room_name_type name{};
		uint8_t flags{};
		room_password_type password{};
		uint8_t max_player_count{};
		datetime create_datetime{};
		client_address host_address{};
		uint8_t current_player_count{};
	};
}
