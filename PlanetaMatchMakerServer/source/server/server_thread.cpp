#include "server_thread.hpp"

#include "logger/log.hpp"
#include "message/message_handler_invoker_factory.hpp"
#include "server_setting.hpp"
#include "server_session.hpp"

namespace pgl {

	server_thread::server_thread(boost::asio::ip::tcp::acceptor& acceptor, std::mutex& acceptor_mutex,
		boost::asio::ssl::context& ssl_context, server_data& server_data, const server_setting& server_setting):
		acceptor_(acceptor),
		acceptor_mutex_(acceptor_mutex),
		ssl_context_(ssl_context),
		server_data_(server_data),
		server_setting_(server_setting),
		message_handler_invoker_(message_handler_invoker_factory::make_shared_standard()) {}

	void server_thread::start() {
		server_sessions_.reserve(server_setting_.common.max_connection_per_thread);
		for (auto i = 0u; i < server_setting_.common.max_connection_per_thread; ++i) {
			auto conn_handler = std::make_shared<server_session>(acceptor_, acceptor_mutex_, ssl_context_, server_data_,
				server_setting_, message_handler_invoker_);
			conn_handler->start();
			server_sessions_.push_back(std::move(conn_handler));
		}
		log(log_level::info, "Start ", server_setting_.common.max_connection_per_thread,
			" connection handlers.");
	}

	void server_thread::stop() {
		for (auto&& session : server_sessions_) { session->stop(); }
		{
			std::lock_guard lock(acceptor_mutex_);
			boost::system::error_code ignored_error;
			acceptor_.cancel(ignored_error);
		}
		server_sessions_.clear();
	}
}
