#include "server_thread.hpp"

#include "message/message_handler_invoker_factory.hpp"
#include "./server_setting.hpp"
#include "./connection_handler.hpp"

namespace pgl {

	server_thread::server_thread(boost::asio::ip::tcp::acceptor& acceptor, server_data& server_data,
		const server_setting& server_setting): acceptor_(acceptor),
		server_data_(server_data),
		server_setting_(server_setting),
		message_handler_invoker_(message_handler_invoker_factory::make_unique_standard()) {}

	void server_thread::start() {
		connection_handlers_.reserve(server_setting_.max_connections_per_thread);
		for (auto i = 0u; i < server_setting_.max_connections_per_thread; ++i) {
			auto conn_handler = std::make_unique<connection_handler>(acceptor_, server_data_,
				server_setting_, *message_handler_invoker_);
			conn_handler->start();
			connection_handlers_.push_back(std::move(conn_handler));
		}
	}

	void server_thread::stop() {
		for (auto&& conn_handler : connection_handlers_) {
			conn_handler->stop();
		}
		connection_handlers_.clear();
	}
}
