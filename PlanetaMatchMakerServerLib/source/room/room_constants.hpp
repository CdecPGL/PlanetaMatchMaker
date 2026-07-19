#pragma once

#include <cstddef>
#include <cstdint>

#include "minimal_serializer/fixed_string.hpp"
#include "utilities/fixed_u8string_validation.hpp"

namespace pgl {
	using room_id_t = uint32_t;
	using room_name_t = minimal_serializer::fixed_u8string<24>; // at least 8 characters with UTF-8
	using room_password_t = minimal_serializer::fixed_u8string<16>; //16 characters with ASCII

	constexpr std::size_t p2p_service_peer_id_bytes = 128;
	using p2p_service_peer_id_t = minimal_serializer::fixed_u8string<p2p_service_peer_id_bytes>;

	[[nodiscard]] inline bool is_valid_p2p_service_peer_id(const p2p_service_peer_id_t& peer_id) {
		return is_canonical_fixed_u8string(peer_id);
	}
}
