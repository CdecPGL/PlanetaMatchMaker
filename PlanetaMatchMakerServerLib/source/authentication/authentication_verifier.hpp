#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <vector>

#include "authentication/authentication_method.hpp"
#include "authentication/authentication_result.hpp"
#include "authentication/authentication_execution_context.hpp"
#include "server/server_setting.hpp"
#include "session/session_data.hpp"

namespace pgl {
	struct authentication_verification_result final {
		authentication_result result = authentication_result::authentication_data_invalid;
		std::optional<authenticated_identity> identity;

		[[nodiscard]] bool succeeded() const { return result == authentication_result::success; }
	};

	class authentication_credential_verifier {
	public:
		virtual ~authentication_credential_verifier() = default;

		[[nodiscard]] virtual authentication_method method() const = 0;

		[[nodiscard]] virtual authentication_verification_result verify(
			const std::vector<uint8_t>& credential,
			const server_authentication_setting& setting,
			const authentication_execution_context& context) const = 0;
	};

	[[nodiscard]] const authentication_credential_verifier* find_authentication_credential_verifier(
		authentication_method method);

	authentication_verification_result verify_authentication_credential(
		authentication_method method,
		const std::vector<uint8_t>& credential,
		const server_authentication_setting& setting,
		const authentication_execution_context& context);
}
