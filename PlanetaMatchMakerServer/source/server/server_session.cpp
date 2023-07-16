#include <boost/asio/spawn.hpp>

#include "message/message_handler_invoker.hpp"
#include "message/messages.hpp"
#include "session/session_data.hpp"
#include "server/server_data.hpp"
#include "room/room_data_container.hpp"
#include "logger/log.hpp"
#include "server_errors.hpp"
#include "utilities/checked_static_cast.hpp"
#include "server/server_setting.hpp"

#include "server_session.hpp"

using namespace std;
using namespace boost;
using namespace minimal_serializer;

namespace pgl {
	server_session::server_session(asio::ip::tcp::acceptor& acceptor, server_data& server_data,
		const server_setting& server_setting, message_handler_invoker& message_handler_invoker):
		acceptor_(acceptor),
		server_data_(server_data),
		server_setting_(server_setting),
		message_handler_invoker_(message_handler_invoker),
		socket_(acceptor.get_executor()) { }

	void server_session::start() {
		// Reset session
		session_data_ = std::make_unique<session_data>();

		// Start connection
		spawn(acceptor_.get_executor(), [shared_this=shared_from_this()](asio::yield_context yield) {
			try {
				log(log_level::debug, "Start to accept.");
				try {
					shared_this->acceptor_.async_accept(shared_this->socket_, yield);
					shared_this->session_data_->set_remote_endpoint(
						endpoint::make_from_boost_endpoint(
							shared_this->socket_.remote_endpoint()));
				}
				catch (system::system_error& e) {
					const auto extra_message = generate_string("Acception failed: ", e, " @",
						shared_this->socket_.remote_endpoint());
					throw server_session_error(extra_message);
				}

				log_with_endpoint(log_level::info, shared_this->socket_.remote_endpoint(),
					"Accepted new connection. Start to receive message.");

				// Prepare data
				const auto message_handler_param = std::make_shared<message_handle_parameter>(message_handle_parameter{
					shared_this->socket_, shared_this->server_data_, yield,
					chrono::seconds(shared_this->server_setting_.common.time_out_seconds),
					*shared_this->session_data_,
					shared_this->server_setting_
				});

				// Authenticate client
				shared_this->message_handler_invoker_.handle_specific_message(message_type::authentication_request,
					message_handler_param, false);

				// Receive message
				while (true) {
					shared_this->message_handler_invoker_.handle_message(message_handler_param,
						shared_this->server_setting_.common.enable_session_key_check);
				}
			}
			catch (const server_session_intended_disconnect_error& e) {
				log_with_endpoint(log_level::info,
					shared_this->session_data_->remote_endpoint().to_boost_endpoint(),
					"Intended disconnect: ", e);
				shared_this->restart();
			}
			catch (const system::system_error& e) {
				log_with_endpoint(log_level::error, shared_this->session_data_->remote_endpoint().to_boost_endpoint(),
					"Unhandled error: ", e, " Disconnect the connection: ");
				shared_this->restart();
			}
			catch (const server_session_error& e) {
				log_with_endpoint(log_level::error, shared_this->session_data_->remote_endpoint().to_boost_endpoint(),
					e, " Disconnect the connection.");
				shared_this->restart();
			}
			catch (const std::exception& e) {
				log_with_endpoint(log_level::error, shared_this->session_data_->remote_endpoint().to_boost_endpoint(),
					typeid(e), ": ", e.what(), " Disconnect the connection.");
				shared_this->restart();
			}
			catch (...) {
				log_with_endpoint(log_level::fatal, shared_this->session_data_->remote_endpoint().to_boost_endpoint(),
					"Unknown error. Stop the server.");
				shared_this->stop();
				throw;
			}
		});
	}

	void server_session::stop() {
		finalize();
		socket_.close();
	}

	void server_session::finalize() const {
		// Remove hosting room if exist
		try { remove_hosting_room_if_need(); }
		catch (...) {
			remove_player_full_name_if_need();
			throw;
		}

		// Remove player name
		remove_player_full_name_if_need();
	}

	void server_session::restart() {
		log(log_level::info, "Server session handler is restarted.");
		stop();
		start();
	}

	void server_session::remove_hosting_room_if_need() const {
		if (session_data_->is_hosting_room()) {
			server_data_.get_room_data_container().try_remove(session_data_->hosting_room_id());
			log_with_endpoint(log_level::info, session_data_->remote_endpoint().to_boost_endpoint(),
				"Hosting room(ID: ", session_data_->hosting_room_id(), ") is removed.");
		}
	}

	void server_session::remove_player_full_name_if_need() const {
		if (const auto& player_name_container = server_data_.get_player_name_container(); player_name_container.
			is_player_exist(session_data_->client_player_name())) {
			server_data_.get_player_name_container().remove_player_name(session_data_->client_player_name());
			log_with_endpoint(log_level::info, session_data_->remote_endpoint().to_boost_endpoint(),
				"Player full name(name: ", session_data_->client_player_name().name, ", Tag: ",
				session_data_->client_player_name().tag, ") is removed.");
		}
	}
}
