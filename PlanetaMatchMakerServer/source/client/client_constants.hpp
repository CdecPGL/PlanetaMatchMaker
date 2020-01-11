#pragma once

#include <cstdint>

#include "minimal_serializer/fixed_string.hpp"

namespace pgl {
	using player_name_t = minimal_serializer::fixed_string<24>; // at least 8 characters with UTF-8
	using player_tag_t = uint16_t;
}
