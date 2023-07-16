#include <vector>

#include <boost/asio.hpp>

#include "server/server_setting.hpp"
#include "session/session_data.hpp"
#include "logger/log.hpp"
#include "../message_parameter_validator.hpp"
#include "connection_test_request_message_handler.hpp"

using namespace std;
using namespace boost;

namespace pgl {
	bool test_connection_tcp(message_handle_parameter& param, const asio::ip::tcp::endpoint& target_endpoint,
		const std::string& test_text) {
		const auto time_out_seconds = std::chrono::seconds(
			param.server_setting.connection_test.connection_check_tcp_time_out_seconds);

		// Try to establish TCP connection
		asio::ip::tcp::socket socket(param.socket.get_executor());
		execute_socket_timed_async_operation(socket, time_out_seconds,
			[&param, &socket, &target_endpoint]() { socket.async_connect(target_endpoint, param.yield); });

		// Send test Text
		execute_socket_timed_async_operation(socket, time_out_seconds,
			[&param, &socket, &test_text]() { async_write(socket, asio::buffer(test_text), param.yield); });

		// Receive reply
		std::string result_text;
		execute_socket_timed_async_operation(socket, time_out_seconds,
			[&param, &socket, &test_text, &result_text]() {
				asio::streambuf buffer;
				async_read(socket, buffer, asio::transfer_exactly(test_text.length()), param.yield);
				result_text = asio::buffer_cast<const char*>(buffer.data());
			});
		socket.close();

		// Check the reply matches test text
		if (result_text != test_text) {
			log_with_endpoint(log_level::info, param.socket.remote_endpoint(), "Connect to ", target_endpoint,
				" successfully, but target endpoint replied reply wrong message:\n expected message is \"", test_text,
				"\", but received \"", result_text, "\".");
			return false;
		}

		log_with_endpoint(log_level::info, param.socket.remote_endpoint(), "Connect to ",
			target_endpoint, " successfully");
		return true;
	}

	bool test_connection_udp(message_handle_parameter& param, const asio::ip::tcp::endpoint& target_endpoint,
		const std::string& test_text) {
		const auto time_out_seconds = std::chrono::seconds(
			param.server_setting.connection_test.connection_check_udp_time_out_seconds);
		const auto try_count = param.server_setting.connection_test.connection_check_udp_try_count;

		// Try to send data by UDP and check if the reply is returned
		asio::ip::udp::socket socket(param.socket.get_executor());
		socket.open(param.socket.local_endpoint().protocol() == asio::ip::tcp::v4()
			            ? asio::ip::udp::v4()
			            : asio::ip::udp::v6());

		// Try several time because it is possible that data lost occurs in UDP
		auto is_succeeded = true;
		for (auto i = 0; i < try_count; ++i) {
			try {
				// Send test Text and receive reply
				std::string result_text;
				execute_socket_timed_async_operation(socket, time_out_seconds,
					[&param, &socket, &target_endpoint, &test_text, &result_text]() {
						auto target_endpoint_udp = asio::ip::udp::endpoint(target_endpoint.address(),
							target_endpoint.port());
						socket.async_send_to(asio::buffer(test_text), target_endpoint_udp, param.yield);

						// std::string::length() returns the length of a string without null character '\0' while the data sent and received includes '\0'.
						// So We need to add 1 byte to std::string::length().
						// Additionally, it is possible to receive data whose end character is not '\0' in UDP (data reception from unexpected host, data corruption in transporting, etc.)
						// In such case, '\0' disappears and we can't determine the end of the string.
						// To avoid unexpected behavior caused by this, we allocate more 1 byte to the buffer and overwrite the extra byte to '\0' after we receive data.
						// Therefore, we set the length of buffer test_text.length() + 2.
						// 
						// send:    |---test_text.length()---|0|       <- '\0' is included.
						// receive: |----------data---------------...| <- it is possible to receive unexpected data in UDP.
						// buffer:  |---test_text.length()---|?|?|     <- the buffer is overwritten by the received data and it is possible that '\0' doesn't exist.
						// buffer:  |---test_text.length()---|?|0|     <- By putting '\0' to the end of the buffer, we avoid lack of null character.
						std::vector<char> buffer_data(test_text.length() + 2);
						const auto buffer = asio::buffer(buffer_data);
						socket.async_receive_from(buffer, target_endpoint_udp, param.yield);
						buffer_data[buffer_data.size() - 1] = '\0';
						result_text = reinterpret_cast<const char*>(buffer_data.data());
					});

				// Check the reply matches test text
				if (result_text != test_text) {
					log_with_endpoint(log_level::info, param.socket.remote_endpoint(), "Connect to ", target_endpoint,
						" successfully, but target endpoint replied reply wrong message:\n expected message is \"",
						test_text, "\", but received \"", result_text, "\". (", i + 1, "/", try_count, " attempts)");
					is_succeeded = false;
				}
				else {
					is_succeeded = true;
					break;
				}
			}
			catch (const system::system_error& e) {
				if (e.code() != asio::error::operation_aborted) { throw; }
				is_succeeded = false;
				log_with_endpoint(log_level::info, param.socket.remote_endpoint(), "Timed out to connect to ",
					target_endpoint, ". (", i + 1, "/", try_count, " attempts)");
			}
		}
		socket.close();

		if (is_succeeded) {
			log_with_endpoint(log_level::info, param.socket.remote_endpoint(), "Connect to ",
				target_endpoint, " successfully");
		}
		else {
			log_with_endpoint(log_level::info, param.socket.remote_endpoint(), "Failed to connect to ",
				target_endpoint, ".");
		}

		return is_succeeded;
	}

	connection_test_request_message_handler::handle_return_t connection_test_request_message_handler::handle_message(
		const connection_test_request_message& message,
		std::shared_ptr<message_handle_parameter> param) {
		const message_parameter_validator parameter_validator(param);

		// Check port number is valid
		parameter_validator.validate_port_number(message.port_number);

		const auto target_endpoint = asio::ip::tcp::endpoint(
			param->session_data.remote_endpoint().to_boost_endpoint().address(), message.port_number);
		log_with_endpoint(log_level::info, param->socket.remote_endpoint(), "Start ", message.protocol,
			" connectable test to ", target_endpoint, " with setting timeout ",
			message.protocol == transport_protocol::tcp
				? param->server_setting.connection_test.connection_check_tcp_time_out_seconds
				: param->server_setting.connection_test.connection_check_udp_time_out_seconds, " seconds.");

		connection_test_reply_message reply{
			true
		};
		try {
			const std::string test_text = "Hello. This is PMMS.";
			switch (message.protocol) {
				case transport_protocol::tcp:
					reply.succeed = test_connection_tcp(*param, target_endpoint, test_text);
					break;
				case transport_protocol::udp:
					reply.succeed = test_connection_udp(*param, target_endpoint, test_text);
					break;
				default:
					const auto error_message = minimal_serializer::generate_string("Indicated protocol \"",
						static_cast<underlying_type_t<transport_protocol>>(message.protocol), "\" is invalid.");
					throw client_error(client_error_code::request_parameter_wrong, false, error_message);
			}
		}
		catch (const system::system_error& e) {
			// disconnection by client is expected behavior (asio::error::eof)
			if (e.code() != asio::error::eof) {
				reply.succeed = false;
				log_with_endpoint(log_level::info, param->socket.remote_endpoint(), "Failed to connect to ",
					target_endpoint, ": ", e, "");
			}
		}

		return {{reply}, false};
	}
}
