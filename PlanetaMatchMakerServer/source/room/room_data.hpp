#pragma once
#include <cstdint>

#include "datetime/datetime.hpp"
#include "client/client_address.hpp"

#include "room_constants.hpp"

namespace pgl {
	struct room_data final {
		room_id_type room_id{};
		uint8_t name[room_name_size]{};
		uint8_t setting_flags{};
		uint8_t password[room_password_size]{};
		uint8_t max_player_count{};
		datetime create_datetime{};
		client_address host_address{};
		uint8_t current_player_count{};
		uint8_t status_flags{};
	};
}
