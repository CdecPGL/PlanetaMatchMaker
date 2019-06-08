#pragma once
#include <cstdint>

namespace pgl {
	struct room_flags_bit_mask final {
		using flags_type = uint8_t;
		const static flags_type is_private = 1;
		const static flags_type is_open = 2;
	};

	using room_id_type = uint32_t;
	using room_name_type = std::array<uint8_t, 24>; // at least 8 characters with UFC-8
	using room_password_type = std::array<uint8_t, 16>; //16 characters with ASCII
}
