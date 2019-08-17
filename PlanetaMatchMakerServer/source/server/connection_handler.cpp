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
		spawn(acceptor_.get_executor(), [shared_this=shared_from_this()](asio::yield_context yield) {
			try {
				log(log_level::debug, "Start to accept.");
				try {
					shared_this->acceptor_.async_accept(shared_this->socket_, yield);
				} catch (system::system_error& e) {
					const auto extra_message = generate_string(e, " @", shared_this->socket_.remote_endpoint());
					throw server_error(server_error_code::acception_failed, extra_message);
				}

				log_with_endpoint(log_level::info, shared_this->socket_.remote_endpoint(),
					"Accepted new connection. Start to receive message.");

				// Prepare data
				const auto message_handler_param = std::make_shared<message_handle_parameter>(message_handle_parameter{
					shared_this->socket_, shared_this->server_data_, yield,
					chrono::seconds(shared_this->server_setting_.time_out_seconds),
					*shared_this->session_data_,
					shared_this->server_setting_
				});

				// Authenticate client
				shared_this->message_handler_invoker_.handle_specific_message(message_type::authentication_request,
					message_handler_param, false);

				// Receive message
				while (true) {
					shared_this->message_handler_invoker_.handle_message(message_handler_param,
						shared_this->server_setting_.enable_session_key_check);
				}
			} catch (const system::system_error& e) {
				log_with_endpoint(log_level::error, shared_this->socket_.remote_endpoint(), "Unhandled error: ", e);
				shared_this->restart();
			}
			catch (const server_error& e) {
				if (e.error_code() == server_error_code::disconnected_expectedly) {
					log_with_endpoint(log_level::info, shared_this->socket_.remote_endpoint(), e);
				} else {
					log_with_endpoint(log_level::error, shared_this->socket_.remote_endpoint(),
						"Message handling error: ", e);
				}

				shared_this->restart();
			}
			catch (const std::exception& e) {
				log_with_endpoint(log_level::fatal, shared_this->socket_.remote_endpoint(), typeid(e), ": ", e.what());
				shared_this->stop();
				throw;
			}
			catch (...) {
				log_with_endpoint(log_level::fatal, shared_this->socket_.remote_endpoint(), "Unknown error.");
				shared_this->stop();
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
