#include <boost/asio/spawn.hpp>

#include "message/message_handler_invoker.hpp"
#include "message/messages.hpp"
#include "session/session_data.hpp"
#include "server/server_data.hpp"
#include "room/room_data_container.hpp"
#include "utilities/log.hpp"
#include "server_error.hpp"
#include "utilities/checked_static_cast.hpp"
#include "server/server_setting.hpp"

#include "connection_handler.hpp"

using namespace std;
using namespace boost;

namespace pgl {
	connection_handler::connection_handler(asio::ip::tcp::acceptor& acceptor, server_data& server_data,
		const server_setting& server_setting, message_handler_invoker& message_handler_invoker):
		acceptor_(acceptor),
		server_data_(server_data),
		server_setting_(server_setting),
		message_handler_invoker_(message_handler_invoker),
		socket_(acceptor.get_executor()) { }

	void connection_handler::start() {
		// Reset session
		session_data_ = std::make_unique<session_data>();

		// Start connection
		spawn(acceptor_.get_executor(), [&](asio::yield_context yield) {
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

				// Prepare data
				const auto message_handler_param = std::make_shared<message_handle_parameter>(message_handle_parameter{
					socket_, server_data_, yield, chrono::seconds(server_setting_.time_out_seconds),
					*session_data_
				});

				// Authenticate client
				message_handler_invoker_.handle_specific_message(message_type::authentication_request,
					message_handler_param, false);

				// Receive message
				while (true) {
					message_handler_invoker_.handle_message(message_handler_param, false);
				}
			} catch (const system::system_error& e) {
				log_with_endpoint(log_level::error, socket_.remote_endpoint(), "Unhandled error: ", e);
				restart();
			}
			catch (const server_error& e) {
				if (e.error_code() == server_error_code::expected_disconnection) {
					log_with_endpoint(log_level::info, socket_.remote_endpoint(), e);
				} else {
					log_with_endpoint(log_level::error, socket_.remote_endpoint(), "Message handling error: ", e);
				}
				restart();
			}
			catch (const std::exception& e) {
				log_with_endpoint(log_level::fatal, socket_.remote_endpoint(), typeid(e), ": ", e.what());
				stop();
				throw;
			}
			catch (...) {
				log_with_endpoint(log_level::fatal, socket_.remote_endpoint(), "Unknown error.");
				stop();
				throw;
			}
		});
	}

	void connection_handler::stop() {
		finalize();
		socket_.close();
	}

	void connection_handler::finalize() const {
		// Remove hosting room if exist
		if (session_data_->is_hosting_room()) {
			server_data_.get_room_data_container(session_data_->hosting_room_group_index()).remove_data(
				session_data_->hosting_room_id());
			log_with_endpoint(log_level::info, socket_.remote_endpoint(), "Hosting room(Group index: ",
				session_data_->hosting_room_group_index(), ", ID: ", session_data_->hosting_room_id(), ") is removed.");
		}
	}

	void connection_handler::restart() {
		log(log_level::info, "Restart server instance.");
		stop();
		start();
	}
}
