#include <boost/asio.hpp>

#include "server/server_setting.hpp"
#include "session/session_data.hpp"
#include "utilities/log.hpp"
#include "../message_handle_utilities.hpp"
#include "connection_test_request_message_handler.hpp"

using namespace boost;

namespace pgl {
	void connection_test_request_message_handler::handle_message(const connection_test_request_message& message,
		std::shared_ptr<message_handle_parameter> param) {

		connection_test_reply_message reply{
			true
		};

		// Try to connect to TCP server in the client
		const auto target_endpoint = asio::ip::tcp::endpoint(
			param->session_data.client_address().get_boost_ip_address(), message.port_number);
		try {
			const auto time_out_seconds = std::chrono::seconds(param->server_setting.connection_check_time_out_seconds);
			execute_timed_async_operation(param->socket, time_out_seconds, [param, &target_endpoint]() {
				asio::io_service io_service;
				asio::ip::tcp::socket socket(io_service);
				socket.async_connect(target_endpoint, param->yield);
				log_with_endpoint(log_level::info, param->socket.remote_endpoint(), "Connect to ", target_endpoint,
					" successfully");
			});
		}
		catch (const system::system_error& e) {
			if (e.code() == asio::error::operation_aborted) {
				reply.succeed = false;
				log_with_endpoint(log_level::info, param->socket.remote_endpoint(), "Failed to connect to ",
					target_endpoint, ".");
			}
			else { throw; }
		}

		reply_message_header header{
			message_type::connection_test_reply,
			message_error_code::ok
		};

		log_with_endpoint(log_level::info, param->socket.remote_endpoint(), "Reply ",
			message_type::connection_test_request, " message.");
		send(param, header, reply);
	}
}
