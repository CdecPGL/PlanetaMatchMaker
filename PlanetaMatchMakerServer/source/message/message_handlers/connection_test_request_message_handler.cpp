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

		const auto time_out_seconds = std::chrono::seconds(param->server_setting.connection_check_time_out_seconds);
		const auto target_endpoint = asio::ip::tcp::endpoint(
			param->session_data.remote_endpoint().to_boost_endpoint().address(), message.port_number);
		log_with_endpoint(log_level::info, param->socket.remote_endpoint(), "Start ", message.protocol,
			" connectable test to ", target_endpoint, " with setting timeout ",
			param->server_setting.connection_check_time_out_seconds, " seconds.");
		try {
			if (message.protocol == transport_protocol::tcp) {
				// Try to connect to TCP server in the client
				asio::ip::tcp::socket socket(param->socket.get_executor());
				execute_socket_timed_async_operation(socket, time_out_seconds, [param,&socket, &target_endpoint]() {
					socket.async_connect(target_endpoint, param->yield);
					socket.close();
					log_with_endpoint(log_level::info, param->socket.remote_endpoint(), "Connect to ", target_endpoint,
						" successfully");
				});
			}
			else {
				// Try to send to UDP client and check if the reply is returned
				asio::ip::udp::socket socket(param->socket.get_executor());
				execute_socket_timed_async_operation(socket, time_out_seconds, [param,&socket, &target_endpoint]() {
					auto target_endpoint_udp = asio::ip::udp::endpoint(target_endpoint.address(),
						target_endpoint.port());
					socket.async_send_to(asio::buffer("Hello."), target_endpoint_udp, param->yield);
					std::array<uint8_t, 16> buffer{};
					socket.async_receive_from(asio::buffer(buffer), target_endpoint_udp, param->yield);
					log_with_endpoint(log_level::info, param->socket.remote_endpoint(), "Connect to ", target_endpoint,
						" successfully");
				});
			}
		}
		catch (const system::system_error& e) {
			// disconnection by client is expected behavior
			if (e.code() != asio::error::eof) {
				reply.succeed = false;
				log_with_endpoint(log_level::info, param->socket.remote_endpoint(), "Failed to connect to ",
					target_endpoint, ".");
			}
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
