#include <boost/asio/spawn.hpp>
#include <exception>
#include <mutex>
#include <utility>

#include "message/message_handler_invoker.hpp"
#include "message/messages.hpp"
#include "async/timer.hpp"
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
	template <typename ... Params>
	void log_with_session_data_endpoint(const log_level level, const session_data& session_data, Params&& ... params) {
		if (const auto session_number = session_data.session_number(); session_number.has_value()) {
			log_with_session_and_endpoint(level, *session_number, session_data.remote_endpoint().to_boost_endpoint(),
				std::forward<Params>(params)...);
		}
		else {
			log_with_endpoint(level, session_data.remote_endpoint().to_boost_endpoint(),
				std::forward<Params>(params)...);
		}
	}

	server_session::server_session(asio::ip::tcp::acceptor& acceptor, std::mutex& acceptor_mutex,
		asio::ssl::context& ssl_context, server_data& server_data, const server_setting& server_setting,
		std::shared_ptr<const message_handler_invoker> message_handler_invoker):
		acceptor_(acceptor),
		acceptor_mutex_(acceptor_mutex),
		ssl_context_(ssl_context),
		server_data_(server_data),
		server_setting_(server_setting),
		message_handler_invoker_(std::move(message_handler_invoker)),
		strand_(asio::make_strand(acceptor.get_executor())),
		connection_(strand_, ssl_context_) { }

	void server_session::start() {
		asio::dispatch(strand_, [shared_this = shared_from_this()] {
			shared_this->start_impl();
		});
	}

	void server_session::start_impl() {
		if (is_stopping_.load(std::memory_order_acquire)) { return; }

		// Reset session
		session_data_ = std::make_unique<session_data>();
		connection_.reset(server_setting_.tls.mode);

		// Start connection
		const auto shared_this = shared_from_this();
		log(log_level::debug, "Start to accept.");
		{
			// Serialize accept initiation on the shared acceptor, then bind completion to this session strand.
			std::lock_guard lock(acceptor_mutex_);
			acceptor_.async_accept(connection_.socket(),
				[shared_this](const system::error_code& accept_error) {
					shared_this->handle_accepted_connection(accept_error);
				});
		}
	}

	void server_session::handle_accepted_connection(const system::error_code& accept_error) {
		if (is_stopping_.load(std::memory_order_acquire)) { return; }

		static_cast<void>(spawn(strand_, [shared_this=shared_from_this(), accept_error](asio::yield_context yield) {
			try {
				try {
					if (shared_this->is_stopping_.load(std::memory_order_acquire)) { return; }
					if (accept_error) {
						const auto extra_message = generate_string("Acception failed: ", accept_error.message());
						throw server_session_error(extra_message);
					}

					shared_this->session_data_->set_remote_endpoint(
						endpoint::make_from_boost_endpoint(
							shared_this->connection_.remote_endpoint()));
					shared_this->session_data_->set_session_number(shared_this->server_data_.issue_session_number());
				}
				catch (system::system_error& e) {
					const auto extra_message = generate_string("Acception failed: ", e, " @",
						shared_this->connection_.remote_endpoint());
					throw server_session_error(extra_message);
				}

				log_with_session_data_endpoint(log_level::info, *shared_this->session_data_,
					"Accepted new connection. Start to receive message.");

				if (shared_this->server_setting_.tls.mode == server_tls_mode::tls) {
					execute_socket_timed_async_operation(shared_this->connection_,
						chrono::seconds(shared_this->server_setting_.common.time_out_seconds),
						[shared_this, &yield]() { shared_this->connection_.async_handshake(yield); });
					log_with_session_data_endpoint(log_level::info, *shared_this->session_data_,
						"TLS handshake completed.");
				}

				// Prepare data
				const auto message_handler_param = std::make_shared<message_handle_parameter>(message_handle_parameter{
					shared_this->connection_, shared_this->server_data_, yield,
					chrono::seconds(shared_this->server_setting_.common.time_out_seconds),
					*shared_this->session_data_,
					shared_this->server_setting_
				});

				// Authenticate client
				shared_this->message_handler_invoker_->handle_specific_message(message_type::authentication,
					message_handler_param);

				// Receive message
				while (true) { shared_this->message_handler_invoker_->handle_message(message_handler_param); }
			}
			catch (const server_session_intended_disconnect_error& e) {
				if (shared_this->is_stopping_.load(std::memory_order_acquire)) { return; }
				log_with_session_data_endpoint(log_level::info, *shared_this->session_data_,
					"Intended disconnect: ", e);
				shared_this->restart();
			}
			catch (const server_session_error& e) {
				if (shared_this->is_stopping_.load(std::memory_order_acquire)) { return; }
				// output log as info for session error because it is caused by external factors like disconnection by client and network error.
				log_with_session_data_endpoint(log_level::info, *shared_this->session_data_,
					e, " Disconnect the connection.");
				shared_this->restart();
			}
			catch (const system::system_error& e) {
				if (shared_this->is_stopping_.load(std::memory_order_acquire)) { return; }
				log_with_session_data_endpoint(log_level::error, *shared_this->session_data_,
					"Unhandled error: ", e, " Disconnect the connection: ");
				shared_this->restart();
			}
			catch (const std::exception& e) {
				if (shared_this->is_stopping_.load(std::memory_order_acquire)) { return; }
				log_with_session_data_endpoint(log_level::error, *shared_this->session_data_,
					typeid(e), ": ", e.what(), " Disconnect the connection.");
				shared_this->restart();
			}
			catch (...) {
				if (shared_this->is_stopping_.load(std::memory_order_acquire)) { return; }
				log_with_session_data_endpoint(log_level::fatal, *shared_this->session_data_,
					"Unknown error. Stop the server.");
				shared_this->stop();
				throw;
			}
		}, asio::detached));
	}

	void server_session::stop() {
		is_stopping_.store(true, std::memory_order_release);
		// dispatch runs inline when already on this strand, preserving restart ordering.
		asio::dispatch(strand_, [shared_this = shared_from_this()] {
			shared_this->stop_impl();
		});
	}

	void server_session::stop_impl() {
		std::exception_ptr finalize_exception;
		if (session_data_) {
			const auto session_data = std::move(session_data_);
			try { finalize(*session_data); }
			catch (...) { finalize_exception = std::current_exception(); }
		}

		boost::system::error_code ignored_error;
		connection_.close(ignored_error);

		if (finalize_exception) { std::rethrow_exception(finalize_exception); }
	}

	void server_session::finalize(const session_data& session_data) const {
		// Remove hosting room if exist
		try { remove_hosting_room_if_need(session_data); }
		catch (...) {
			remove_player_full_name_if_need(session_data);
			throw;
		}

		// Remove player name
		remove_player_full_name_if_need(session_data);
	}

	void server_session::restart() {
		if (is_stopping_.load(std::memory_order_acquire)) { return; }
		if (session_data_) {
			log_with_session_data_endpoint(log_level::info, *session_data_, "Server session handler is restarted.");
		}
		else { log(log_level::info, "Server session handler is restarted."); }
		asio::dispatch(strand_, [shared_this = shared_from_this()] {
			shared_this->stop_impl();
			shared_this->start_impl();
		});
	}

	void server_session::remove_hosting_room_if_need(const session_data& session_data) const {
		if (session_data.is_hosting_room()) {
			server_data_.get_room_data_container().try_remove(session_data.hosting_room_id());
			log_with_session_data_endpoint(log_level::info, session_data,
				"Hosting room(ID: ", session_data.hosting_room_id(), ") is removed.");
		}
	}

	void server_session::remove_player_full_name_if_need(const session_data& session_data) const {
		if (const auto& player_name_container = server_data_.get_player_name_container(); player_name_container.
			is_player_exist(session_data.client_player_name())) {
			server_data_.get_player_name_container().remove_player_name(session_data.client_player_name());
			log_with_session_data_endpoint(log_level::info, session_data,
				"Player full name(name: ", session_data.client_player_name().name, ", Tag: ",
				session_data.client_player_name().tag, ") is removed.");
		}
	}
}
