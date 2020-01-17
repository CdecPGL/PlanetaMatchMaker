#include <boost/asio.hpp>

#include "server/server_data.hpp"
#include "server/server_constants.hpp"
#include "server/server_session_error.hpp"
#include "session/session_data.hpp"
#include "logger/log.hpp"
#include "../message_handle_utilities.hpp"
#include "authentication_request_message_handler.hpp"

using namespace boost;

namespace pgl {
	void authentication_request_message_handler::handle_message(const authentication_request_message& message,
		std::shared_ptr<message_handle_parameter> param) {

		authentication_reply_message reply{
			api_version,
			{},
			{}
		};

		// Check if the client version matches the server version. If not, send an error to the client
		if (message.version != api_version) {
			reply_message_header header{
				message_type::authentication_reply,
				message_error_code::api_version_mismatch,
			};
			send(param, header, reply);
			const auto error_message = minimal_serializer::generate_string(
				"Authentication failed. The client api version doesn't match to the server api version. (server api version: ",
				api_version, ", client api version: ", message.version, ")");
			throw server_session_error(server_session_error_code::not_continuable_error, error_message);
		}
		log_with_endpoint(log_level::info, param->socket.remote_endpoint(), "Authentication succeeded.");

		// Check if player name is valid (not empty)
		if (message.player_name.length() == 0) {
			reply_message_header header{
				message_type::authentication_reply,
				message_error_code::request_parameter_wrong,
			};
			send(param, header, reply);
			throw server_session_error(server_session_error_code::not_continuable_error,
				"Authentication failed. The player name is empty.");
		}

		// Generate session key
		if (param->session_data.is_session_key_generated()) {
			throw server_session_error(server_session_error_code::not_continuable_error,
				"A session key is already generated. Multi time authentication is not allowed.");
		}
		reply.session_key = param->session_data.generate_session_key();
		log_with_endpoint(log_level::info, param->socket.remote_endpoint(), "A session key(", reply.session_key,
			") is generated.");

		// Generate player full name
		const auto player_full_name = param->server_data.get_player_name_container().assign_player_name(
			message.player_name);
		log_with_endpoint(log_level::info, param->socket.remote_endpoint(), "A player \"", player_full_name.name,
			"\" is registered with tag \"", player_full_name.tag, "\"");
		param->session_data.set_client_player_name(player_full_name);
		reply.player_tag = player_full_name.tag;

		// Reply to the client
		reply_message_header header{
			message_type::authentication_reply,
			message_error_code::ok,
		};
		log_with_endpoint(log_level::info, param->socket.remote_endpoint(), "Reply ",
			message_type::authentication_request, " message.");
		send(param, header, reply);
	}
}
