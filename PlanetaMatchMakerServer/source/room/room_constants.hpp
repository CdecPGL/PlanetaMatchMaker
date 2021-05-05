#pragma once

#include <cstdint>

#include "minimal_serializer/fixed_string.hpp"

namespace pgl {
	using room_id_t = uint32_t;
	using room_name_t = minimal_serializer::fixed_string<24>; // at least 8 characters with UTF-8
	using room_password_t = minimal_serializer::fixed_string<16>; //16 characters with ASCII
}
