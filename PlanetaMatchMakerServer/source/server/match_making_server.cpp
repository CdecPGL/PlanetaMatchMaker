#include <boost/asio/spawn.hpp>
#include <utility>

#include "nameof.hpp"

#include "message/message_handler_invoker.hpp"
#include "message/messages.hpp"
#include "utilities/log.hpp"
#include "match_making_server.hpp"
#include "server_error.hpp"
#include "utilities/checked_static_cast.hpp"

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
		log(log_level::info, "Server instance is generated with IP ", ip_version, " and port ", port_number, ".");
	}

	void match_making_server::start() {
		spawn(io_service_, [&](asio::yield_context yield) {
			try {
				log(log_level::debug, "Start to accept.");
				try {
					acceptor_.async_accept(socket_, yield);
				} catch (system::system_error& e) {
					const auto extra_message = generate_string(e, " @", socket_.remote_endpoint());
					throw server_error(server_error_code::acception_failed, extra_message);
				}

				log_with_endpoint(log_level::info, socket_.remote_endpoint(),
					"Accepted new connection. Start to receive message.");

				// Authenticate client
				const auto message_handler_param = std::make_shared<message_handle_parameter>(message_handle_parameter{
					io_service_, socket_, receive_buff_, server_data_, yield, chrono::seconds(time_out_seconds_)
				});
				message_handler_container_->handle_specific_message(message_type::authentication_request,
					message_handler_param);

				// Receive message
				while (true) {
					message_handler_container_->handle_message(message_handler_param);
				}
			} catch (const system::system_error& e) {
				log_with_endpoint(log_level::error, socket_.remote_endpoint(), "Unhandled error: ", e);
				restart();
			}
			catch (const server_error& e) {
				log_with_endpoint(log_level::error, socket_.remote_endpoint(), "Message handling error: ", e);
				restart();
			}catch (const static_cast_range_error& e) {
				log_with_endpoint(log_level::error, socket_.remote_endpoint(), "static_cast error: ", e.what());
				throw;
			}
			catch (const std::exception& e) {
				log_with_endpoint(log_level::fatal, socket_.remote_endpoint(), "Unknown error: ", e.what());
				throw;
			}
			catch (...) {
				log_with_endpoint(log_level::fatal, socket_.remote_endpoint(), "Unknown error.");
				throw;
			}
		});
	}

	void match_making_server::restart() {
		log(log_level::error, "Restart server instance.");
		socket_.close();
		start();
	}
}
