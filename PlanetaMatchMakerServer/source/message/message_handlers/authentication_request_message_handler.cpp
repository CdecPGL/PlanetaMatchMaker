#include <boost/asio.hpp>

#include "server/server_data.hpp"
#include "server/server_constants.hpp"
#include "server/server_error.hpp"
#include "session/session_data.hpp"
#include "utilities/log.hpp"
#include "../message_handle_utilities.hpp"
#include "authentication_request_message_handler.hpp"

using namespace boost;

namespace pgl {
	void authentication_request_message_handler::handle_message(const authentication_request_message& message,
		std::shared_ptr<message_handle_parameter> param) {

		authentication_reply_message reply{
			server_version,
			{}
		};

		// Check if the client version matches the server version. If not, send an error to the client
		if (message.version != server_version) {
			log_with_endpoint(log_level::error, param->socket.remote_endpoint(),
				"Authentication failed. The client version doesn't match to the server version. (server version: ",
				server_version,
				", client version: ", message.version, ")");

			reply_message_header header{
				message_type::authentication_reply,
				message_error_code::version_mismatch,
			};
			send(param, header, reply);
			throw server_error(server_error_code::version_mismatch);
		}
		log_with_endpoint(log_level::info, param->socket.remote_endpoint(), "Authentication succeeded.");

		// Generate session key
		if (param->session_data.is_session_key_generated()) {
			throw server_error(server_error_code::invalid_session,
				"A session key is already generated. Multi time authentication is not allowed.");
		}
		reply.session_key = param->session_data.generate_session_key();
		log_with_endpoint(log_level::info, param->socket.remote_endpoint(), "A session key(", reply.session_key,
			") is generated.");

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
