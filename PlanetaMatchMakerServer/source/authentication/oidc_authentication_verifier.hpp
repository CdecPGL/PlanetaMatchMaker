#pragma once

#include <memory>

#include "authentication/authentication_verifier.hpp"

namespace pgl {
	std::unique_ptr<authentication_credential_verifier> make_oidc_authentication_verifier();
}
