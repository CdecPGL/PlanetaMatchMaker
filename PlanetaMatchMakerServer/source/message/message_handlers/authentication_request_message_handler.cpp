#include <boost/asio.hpp>

#include "server/server_data.hpp"
#include "server/server_constants.hpp"
#include "session/session_data.hpp"
#include "logger/log.hpp"
#include "authentication_request_message_handler.hpp"
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

		// Check if the client version matches the server version. If not, send an error to the client
		if (message.version != api_version) {
			const auto error_message = generate_string(
				"Authentication failed. The client api version doesn't match to the server api version. (server api version: ",
				api_version, ", client api version: ", message.version, ")");
			throw client_error(client_error_code::api_version_mismatch, true, error_message);
		}

		log_with_endpoint(log_level::info, param->socket.remote_endpoint(), "Authentication succeeded.");

		authentication_reply_message reply{
			api_version,
			{},
			{}
		};

		// Generate player full name
		const auto player_full_name = param->server_data.get_player_name_container().assign_player_name(
			message.player_name);
		log_with_endpoint(log_level::info, param->socket.remote_endpoint(), "A player \"", player_full_name.name,
			"\" is registered with tag \"", player_full_name.tag, "\"");
		param->session_data.set_client_player_name(player_full_name);
		reply.player_tag = player_full_name.tag;

		// Mark as authenticated
		param->session_data.set_authenticated();

		// Reply to the client
		return {{reply}, false};
	}
}
