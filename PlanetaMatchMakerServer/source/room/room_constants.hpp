#pragma once

#include <cstdint>

#include "minimal_serializer/fixed_string.hpp"

namespace pgl {
	using room_group_index_type = uint8_t;
	using room_id_type = uint32_t;
	using room_name_type = minimal_serializer::fixed_string<24>; // at least 8 characters with UFC-8
	using room_password_type = minimal_serializer::fixed_string<16>; //16 characters with ASCII
	using room_group_name_type = minimal_serializer::fixed_string<24>; // at least 8 characters with UTF-8

	constexpr int room_group_max_count = 10;
}
