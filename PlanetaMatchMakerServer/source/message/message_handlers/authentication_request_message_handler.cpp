#include <boost/asio.hpp>

#include "server/server_data.hpp"
#include "server/server_constants.hpp"
#include "server/server_error.hpp"
#include "utilities/log.hpp"
#include "../message_handle_utilities.hpp"
#include "authentication_request_message_handler.hpp"

using namespace boost;

namespace pgl {
	void authentication_request_message_handler::handle_message(const authentication_request_message& message,
		std::shared_ptr<message_handle_parameter> param) {
		// Reply version when both not error and error
		const authentication_reply_message reply{
			server_version
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

		// Register or update the client data to the server
		const client_data client_data{
			client_address::make_from_endpoint(param->socket.remote_endpoint()),
			datetime::now()
		};
		const auto client_address = client_address::make_from_endpoint(param->socket.remote_endpoint());
		if (param->server_data->client_data_container().is_data_exist(client_address)) {
			param->server_data->client_data_container().update_data(client_address, client_data);
			log_with_endpoint(log_level::info, param->socket.remote_endpoint(), "Client data updated.");
		} else {
			param->server_data->client_data_container().add_data(client_address, client_data);
			log_with_endpoint(log_level::info, param->socket.remote_endpoint(), "Client data registered.");
		}

		// Reply to the client
		reply_message_header header{
			message_type::authentication_reply,
			message_error_code::ok,
		};
		send(param, header, reply);
		log_with_endpoint(log_level::info, param->socket.remote_endpoint(), "Reply authentication message.");
	}
}
