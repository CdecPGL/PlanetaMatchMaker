#include <boost/asio/spawn.hpp>
#include <utility>

#include "nameof.hpp"

#include "message/message_handler_invoker.hpp"
#include "message/messages.hpp"
#include "utilities/io_utility.hpp"
#include "utilities/asio_stream_compatibility.hpp"
#include "match_making_server.hpp"
#include "server_error.hpp"

using namespace std;
using namespace boost;

namespace pgl {
	constexpr int buffer_size{256};

	match_making_server::
	match_making_server(std::shared_ptr<server_data> server_data,
	                    std::shared_ptr<message_handler_invoker> message_handler_container,
	                    asio::io_service& io_service, const ip_version ip_version,
	                    const std::uint16_t port_number,
	                    const std::uint32_t time_out_seconds)
		: server_data_(std::move(server_data)),
		  message_handler_container_(std::move(message_handler_container)),
		  io_service_(io_service),
		  acceptor_(io_service, asio::ip::tcp::endpoint(get_tcp(ip_version), port_number)),
		  socket_(io_service),
		  receive_buff_(buffer_size),
		  timer_(io_service),
		  time_out_seconds_(time_out_seconds) {
		print_line("Server instance is generated with ip version ", NAMEOF_ENUM(ip_version), " and port number ",
		           port_number, ".");
	}

	void match_making_server::start() {
		spawn(io_service_, [&](asio::yield_context yield)
		{
			try {
				print_line("Start to accept.");
				try {
					acceptor_.async_accept(socket_, yield);
				} catch (system::system_error& e) {
					const auto extra_message = generate_string(e, " @", socket_.remote_endpoint());
					throw server_error(server_error_code::acception_failed, extra_message);
				}

				print_line("Accepted new connection. Start to receive message. @", socket_.remote_endpoint());

				// Authenticate client
				message_handle_parameter message_handler_param{
					io_service_, socket_, receive_buff_, server_data_, yield, chrono::seconds(time_out_seconds_)
				};
				message_handler_container_->handle_specific_message(message_type::authentication_request,
				                                                    message_handler_param);

				// Receive message
				while (true) {
					message_handler_container_->handle_message(message_handler_param);
				}
			} catch (system::system_error& e) {
				print_error_line("Unhandled error @", socket_.remote_endpoint(), ": ", e);
				restart();
			}
			catch (server_error& e) {
				print_error_line("Message handling error @", socket_.remote_endpoint(), ": ", e);
				restart();
			}
		});
	}

	void match_making_server::restart() {
		print_error_line("Restart server instance.");
		socket_.close();
		start();
	}
}
