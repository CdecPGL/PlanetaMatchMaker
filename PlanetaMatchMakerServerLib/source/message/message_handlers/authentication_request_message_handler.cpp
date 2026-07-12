#include <algorithm>
#include <vector>

#include <boost/asio.hpp>

#include "authentication/authentication_verifier.hpp"
#include "server/server_data.hpp"
#include "server/server_constants.hpp"
#include "session/session_data.hpp"
#include "logger/log.hpp"
#include "authentication_request_message_handler.hpp"
#include "server/server_setting.hpp"
#include "authentication/game.hpp"
#include "../message_parameter_validator.hpp"

using namespace boost;
using namespace minimal_serializer;
using namespace std::string_literals;

namespace pgl {
	namespace {
		authentication_request_message_handler::handle_return_t make_authentication_failure_reply(
			const authentication_result result,
			const api_version_type reply_api_version,
			const game_version_t& server_game_version) {
			return {
				{{result, reply_api_version, server_game_version, 0}},
				true
			};
		}
	}

	authentication_request_message_handler::handle_return_t authentication_request_message_handler::handle_message(
		const authentication_request_message& message,
		const std::shared_ptr<message_handle_parameter> param) {
		const message_parameter_validator parameter_validator(param);
		// Check status
		if (param->session_data.is_authenticated()) {
			const auto error_message =
				"A session is already authenticated. Multiple time authentication is not allowed."s;
			throw client_error(client_error_code::operation_invalid, true, error_message);
		}

		// Check if player name is valid
		parameter_validator.validate_player_name(message.player_name, false);

		const auto server_game_version = game_version_t(param->server_setting.authentication.game_version);

		// Check if the client api version matches the server api version. If not, reply authentication failure and disconnect the client
		if (message.api_version != api_version) {
			log_with_session(log_level::info, param,
				"Authentication failed. The client api version doesn't match to the server api version. (server api version: ",
				api_version, ", client api version: ", message.api_version, ")");
			return make_authentication_failure_reply(authentication_result::api_version_mismatch, api_version,
				server_game_version);
		}

		// Check if the client game id matches the server game id. If not, reply authentication failure and disconnect the client
		if (const auto server_game_id = game_id_t(param->server_setting.authentication.game_id); message.game_id !=
			server_game_id) {
			log_with_session(log_level::info, param,
				"Authentication failed. The client game id doesn't match to the server id version. (server game version: ",
				server_game_id, ", client game version: ", message.game_id, ")");
			return make_authentication_failure_reply(authentication_result::game_id_mismatch, api_version,
				server_game_version);
		}

		// Check if the client game version matches the server game version if game version check is enabled. If not, reply authentication failure and disconnect the client
		if (param->server_setting.authentication.enable_game_version_check && message.game_version !=
			server_game_version) {
			log_with_session(log_level::info, param,
				"Authentication failed. The client game version doesn't match to the server game version. (server game version: ",
				server_game_version, ", client game version: ", message.game_version, ")");
			return make_authentication_failure_reply(authentication_result::game_version_mismatch, api_version,
				server_game_version);
		}

		if (!param->server_setting.authentication.is_method_enabled(message.authentication_method)) {
			log_with_session(log_level::info, param, "Authentication failed. Unsupported authentication method: ",
				message.authentication_method);
			return make_authentication_failure_reply(authentication_result::unsupported_authentication_method,
				api_version, server_game_version);
		}

		if (!param->connection.is_tls() && !param->server_setting.authentication.allow_plain_connections) {
			log_with_session(log_level::warning, param,
				"Authentication failed because authentication over plain TCP is not allowed.");
			return make_authentication_failure_reply(authentication_result::insecure_connection, api_version,
				server_game_version);
		}

		if (message.credential_size == 0) {
			log_with_session(log_level::info, param, "Authentication failed. Credential is empty.");
			return make_authentication_failure_reply(authentication_result::authentication_data_format_invalid,
				api_version, server_game_version);
		}

		if (message.credential_size > authentication_max_credential_bytes ||
			message.credential_size > param->server_setting.authentication.max_credential_bytes) {
			log_with_session(log_level::info, param,
				"Authentication failed. Credential size exceeds configured limit. (limit: ",
				param->server_setting.authentication.max_credential_bytes, ", actual: ", message.credential_size,
				")");
			return make_authentication_failure_reply(authentication_result::authentication_data_size_exceeded,
				api_version, server_game_version);
		}

		std::vector<uint8_t> credential;
		credential.reserve(message.credential_size);
		const auto chunk_count = (message.credential_size + authentication_credential_chunk_data_size - 1) /
			authentication_credential_chunk_data_size;
		for (auto sequence = 0u; sequence < chunk_count; ++sequence) {
			authentication_credential_chunk_message chunk{};
			receive(param, chunk);
			const auto remaining = message.credential_size - credential.size();
			const auto expected_data_size = std::min<std::size_t>(authentication_credential_chunk_data_size, remaining);
			if (chunk.sequence != sequence || chunk.data_size != expected_data_size) {
				log_with_session(log_level::info, param,
					"Authentication failed. Credential chunk is invalid. (expected sequence: ", sequence,
					", actual sequence: ", chunk.sequence, ", expected size: ", expected_data_size,
					", actual size: ", static_cast<uint16_t>(chunk.data_size), ")");
				return make_authentication_failure_reply(authentication_result::authentication_data_format_invalid,
					api_version, server_game_version);
			}
			credential.insert(credential.end(), chunk.data.begin(), chunk.data.begin() + chunk.data_size);
		}

		const authentication_execution_context authentication_context{
			param->connection.get_executor(),
			param->yield,
			std::chrono::steady_clock::now() +
			std::chrono::seconds(param->server_setting.authentication.timeout_seconds)
		};
		const auto verification_result = verify_authentication_credential(message.authentication_method, credential,
			message.player_name, param->server_setting.authentication, authentication_context);
		if (!verification_result.succeeded() || !verification_result.identity.has_value()) {
			log_with_session(log_level::info, param, "Authentication failed. (method: ",
				message.authentication_method, ", result: ", static_cast<int>(verification_result.result), ")");
			return make_authentication_failure_reply(verification_result.result, api_version, server_game_version);
		}

		log_with_session(log_level::info, param, "Authentication succeeded.");

		// Generate player full name
		const auto player_full_name = param->server_data.get_player_name_container().assign_player_name(
			message.player_name);
		log_with_session(log_level::info, param, "A player \"", player_full_name.name,
			"\" is registered with tag \"", player_full_name.tag, "\"");
		param->session_data.set_client_player_name(player_full_name);

		// Mark as authenticated
		param->session_data.set_authenticated(*verification_result.identity);

		// Reply to the client
		authentication_reply_message reply{
			authentication_result::success, api_version, server_game_version, player_full_name.tag
		};
		return {{reply}, false};
	}
}
