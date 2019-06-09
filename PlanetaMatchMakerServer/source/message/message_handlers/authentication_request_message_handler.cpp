#include <boost/asio.hpp>

#include "server/server_constants.hpp"
#include "server/server_error.hpp"
#include "async/timer.hpp"
#include "async/read_write.hpp"
#include "utilities/string_utility.hpp"

#include "authentication_request_message_handler.hpp"
#include "utilities/log.hpp"

using namespace boost;

namespace pgl {
	void authentication_request_message_handler::handle_message(const authentication_request_message& message,
	                                                            message_handle_parameter& param) {
		try {
			reply_message_header header{
				message_type::authentication_reply,
				message_error_code::ok,
			};

			authentication_reply_message reply{
				server_version
			};

			if (message.version == server_version) {
				log_with_endpoint(log_level::info, param.socket.remote_endpoint(), "Authentication succeeded.");
			} else {
				log_with_endpoint(log_level::error, param.socket.remote_endpoint(), "Authentication failed.");
				header.error_code = message_error_code::version_mismatch;
			}

			execute_timed_async_operation(param.io_service, param.socket, param.timeout_seconds, [&]()
			{
				packed_async_write(param.socket, param.yield, header, reply);
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
