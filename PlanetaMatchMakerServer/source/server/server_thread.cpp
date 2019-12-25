#include "server_thread.hpp"

#include "utilities/log.hpp"
#include "message/message_handler_invoker_factory.hpp"
#include "server_setting.hpp"
#include "server_session.hpp"

namespace pgl {

	server_thread::server_thread(boost::asio::ip::tcp::acceptor& acceptor, server_data& server_data,
		const server_setting& server_setting): acceptor_(acceptor),
		server_data_(server_data),
		server_setting_(server_setting),
		message_handler_invoker_(message_handler_invoker_factory::make_unique_standard()) {}

	void server_thread::start() {
		server_sessions_.reserve(server_setting_.max_connection_per_thread);
		for (auto i = 0u; i < server_setting_.max_connection_per_thread; ++i) {
			auto conn_handler = std::make_shared<server_session>(acceptor_, server_data_,
				server_setting_, *message_handler_invoker_);
			conn_handler->start();
			server_sessions_.push_back(std::move(conn_handler));
		}
		log(log_level::info, "Start ", server_setting_.max_connection_per_thread, " connection handlers.");
	}

	void server_thread::stop() {
		for (auto&& session : server_sessions_) {
			session->stop();
		}
		server_sessions_.clear();
	}
}
