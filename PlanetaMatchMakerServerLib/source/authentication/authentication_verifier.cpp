#include "authentication/authentication_verifier.hpp"

#include <vector>

#include "authentication/steam_authentication_verifier.hpp"

namespace pgl {
	namespace {
		authentication_verification_result failure(const authentication_result result) {
			return {result, std::nullopt};
		}

		const std::vector<std::unique_ptr<authentication_credential_verifier>>& verifier_registry() {
			static const auto registry = [] {
				std::vector<std::unique_ptr<authentication_credential_verifier>> verifiers;
				verifiers.push_back(make_steam_authentication_verifier());
				return verifiers;
			}();
			return registry;
		}
	}

	const authentication_credential_verifier* find_authentication_credential_verifier(
		const authentication_method method) {
		for (const auto& verifier : verifier_registry()) {
			if (verifier->method() == method) { return verifier.get(); }
		}
		return nullptr;
	}

	authentication_verification_result verify_authentication_credential(
		const authentication_method method,
		const std::vector<uint8_t>& credential,
		const server_authentication_setting& setting,
		const authentication_execution_context& context) {
		const auto* verifier = find_authentication_credential_verifier(method);
		if (verifier == nullptr) {
			return failure(authentication_result::unsupported_authentication_method);
		}
		return verifier->verify(credential, setting, context);
	}
}
