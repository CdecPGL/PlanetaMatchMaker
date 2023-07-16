#pragma once

#include "minimal_serializer/fixed_string.hpp"

namespace pgl {
	constexpr auto game_version_bytes = 24;
	using game_version_t = minimal_serializer::fixed_u8string<game_version_bytes>; // at least 8 characters with UTF-8
	constexpr auto game_id_bytes = 24;
	using game_id_t = minimal_serializer::fixed_u8string<game_id_bytes>; // at least 8 characters with UTF-8
}
