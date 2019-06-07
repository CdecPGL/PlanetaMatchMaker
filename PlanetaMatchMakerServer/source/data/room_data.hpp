#pragma once
#include <cstdint>

#include "message/messages.hpp"

namespace pgl {
	struct room_data final {
		room_id_type room_id;
		uint8_t name[room_name_size];
		uint8_t setting_flags;
		uint8_t password[room_password_size];
		uint8_t max_player_count;
		uint64_t date;
		uint32_t host_ip_v4_address;
		uint64_t host_ip_v6_address[2];
		uint16_t host_port_number;
		uint8_t current_player_count;
		uint8_t status_flags;
	};
}
