#include <boost/asio.hpp>

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
			log_with_endpoint(log_level::info, param->socket.remote_endpoint(),
				"Authentication failed. The client api version doesn't match to the server api version. (server api version: ",
				api_version, ", client api version: ", message.api_version, ")");
			return {
				{
					{
						authentication_result::api_version_mismatch, api_version, server_game_version, 0
					},
				},
				true
			};
		}

		// Check if the client game id matches the server game id. If not, reply authentication failure and disconnect the client
		if (const auto server_game_id = game_id_t(param->server_setting.authentication.game_id); message.game_id !=
			server_game_id) {
			log_with_endpoint(log_level::info, param->socket.remote_endpoint(),
				"Authentication failed. The client game id doesn't match to the server id version. (server game version: ",
				server_game_id, ", client game version: ", message.game_id, ")");
			return {
				{
					{
						authentication_result::game_id_mismatch, api_version, server_game_version, 0
					},
				},
				true
			};
		}

		// Check if the client game version matches the server game version if game version check is enabled. If not, reply authentication failure and disconnect the client
		if (param->server_setting.authentication.enable_game_version_check && message.game_version !=
			server_game_version) {
			log_with_endpoint(log_level::info, param->socket.remote_endpoint(),
				"Authentication failed. The client game version doesn't match to the server game version. (server game version: ",
				server_game_version, ", client game version: ", message.game_version, ")");
			return {
				{
					{
						authentication_result::game_version_mismatch, api_version, server_game_version, 0
					},
				},
				true
			};
		}

		log_with_endpoint(log_level::info, param->socket.remote_endpoint(), "Authentication succeeded.");

		// Generate player full name
		const auto player_full_name = param->server_data.get_player_name_container().assign_player_name(
			message.player_name);
		log_with_endpoint(log_level::info, param->socket.remote_endpoint(), "A player \"", player_full_name.name,
			"\" is registered with tag \"", player_full_name.tag, "\"");
		param->session_data.set_client_player_name(player_full_name);

		// Mark as authenticated
		param->session_data.set_authenticated();

		// Reply to the client
		authentication_reply_message reply{
			authentication_result::success, api_version, server_game_version, player_full_name.tag
		};
		return {{reply}, false};
	}
}
