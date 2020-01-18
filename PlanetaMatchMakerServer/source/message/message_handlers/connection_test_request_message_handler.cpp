#include <boost/asio.hpp>

#include "server/server_setting.hpp"
#include "session/session_data.hpp"
#include "logger/log.hpp"
#include "../message_parameter_validator.hpp"
#include "connection_test_request_message_handler.hpp"

using namespace std;
using namespace boost;

namespace pgl {
	void connection_test_request_message_handler::handle_message(const connection_test_request_message& message,
		std::shared_ptr<message_handle_parameter> param) {

		const message_parameter_validator_with_reply<message_type::connection_test_reply, connection_test_reply_message>
			parameter_validator(param);

		// Check port number is valid
		parameter_validator.validate_port_number(message.port_number);

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
			switch (message.protocol) {
				case transport_protocol::tcp: {
					// Try to establish TCP connection
					asio::ip::tcp::socket socket(param->socket.get_executor());
					execute_socket_timed_async_operation(socket, time_out_seconds,
						[param, &socket, &target_endpoint]() {
							socket.async_connect(target_endpoint, param->yield);
							socket.close();
							log_with_endpoint(log_level::info, param->socket.remote_endpoint(), "Connect to ",
								target_endpoint,
								" successfully");
						});
				}
				break;
				case transport_protocol::udp: {
					// Try to send data by UDP and check if the reply is returned
					asio::ip::udp::socket socket(param->socket.get_executor());
					socket.open(param->socket.local_endpoint().protocol() == asio::ip::tcp::v4()
						            ? asio::ip::udp::v4()
						            : asio::ip::udp::v6());
					execute_socket_timed_async_operation(socket, time_out_seconds,
						[param, &socket, &target_endpoint]() {
							auto target_endpoint_udp = asio::ip::udp::endpoint(target_endpoint.address(),
								target_endpoint.port());
							socket.async_send_to(asio::buffer("Hello."), target_endpoint_udp, param->yield);
							// check only if data received. We don't check content of data.
							std::array<uint8_t, 16> buffer{};
							socket.async_receive_from(asio::buffer(buffer), target_endpoint_udp, param->yield);
							log_with_endpoint(log_level::info, param->socket.remote_endpoint(), "Connect to ",
								target_endpoint,
								" successfully");
						});
				}
				break;
				default:
					reply_message_header header{
						message_type::connection_test_reply,
						message_error_code::request_parameter_wrong
					};
					reply.succeed = false;
					send(param, header, reply);
					const auto error_message = minimal_serializer::generate_string("Indicated protocol \"",
						static_cast<underlying_type_t<transport_protocol>>(message.protocol), "\" is invalid.");
					throw server_session_error(server_session_error_code::continuable_error, error_message);
			}
		}
		catch (const system::system_error& e) {
			// disconnection by client is expected behavior
			if (e.code() != asio::error::eof) {
				reply.succeed = false;
				log_with_endpoint(log_level::info, param->socket.remote_endpoint(), "Failed to connect to ",
					target_endpoint, ": ", e, "");
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
