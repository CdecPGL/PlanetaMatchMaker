#pragma once

#include <memory>

#include "authentication/authentication_verifier.hpp"
#include "room/room_constants.hpp"

namespace pgl {
	std::unique_ptr<authentication_credential_verifier> make_steam_authentication_verifier();

	[[nodiscard]] p2p_service_peer_id_t derive_steam_p2p_service_peer_id(
		const authentication_provider_user_id_t& authentication_provider_user_id);
}
