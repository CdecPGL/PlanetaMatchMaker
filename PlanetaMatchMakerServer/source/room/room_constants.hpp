#pragma once
#include <cstdint>

namespace pgl {
	constexpr int room_name_size = 24; // at least 8 characters with UFC-8
	constexpr int room_password_size = 16; //16 characters with ASCII

	struct room_flags_bit_mask final {
		using flags_type = uint8_t;
		const static flags_type is_private = 1;
		const static flags_type is_open = 2;
	};

	using room_id_type = uint32_t;
}
