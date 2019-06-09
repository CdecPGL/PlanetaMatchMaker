#include <boost/asio.hpp>

#include "server/server_constants.hpp"
#include "server/server_error.hpp"
#include "async/timer.hpp"
#include "utilities/string_utility.hpp"

#include "authentication_request_message_handler.hpp"
#include "utilities/log.hpp"

using namespace boost;

namespace pgl {
	void authentication_request_message_handler::handle_message(const authentication_request_message& message,
	                                                            message_handle_parameter& param) {
		try {
			authentication_reply_message reply{
				authentication_reply_message::error_code::ok,
				server_version
			};
			if (message.version == server_version) {
				log_with_endpoint(log_level::info, param.socket.remote_endpoint(), "Authentication succeeded.");
			} else {
				log_with_endpoint(log_level::error, param.socket.remote_endpoint(), "Authentication failed.");
				reply.error_code = authentication_reply_message::error_code::version_mismatch;
			}

			execute_timed_async_operation(param.io_service, param.socket, param.timeout_seconds, [&param, &reply]()
			{
				async_write(param.socket, asio::buffer(&reply, sizeof(reply)), param.yield);
			});
			log_with_endpoint(log_level::info, param.socket.remote_endpoint(), "Reply authentication message.");
		} catch (const system::system_error& e) {
			throw server_error(server_error_code::message_send_error, e.code().message());
		}

		if (message.version != server_version) {
			const auto extra_message = generate_string("server version: ", server_version, ", client version: ",
			                                           message.version);
			throw server_error(server_error_code::version_mismatch, extra_message);
		}
	}
}
