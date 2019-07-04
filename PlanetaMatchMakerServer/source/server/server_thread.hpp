#pragma once

#include <memory>
#include <vector>

#include <boost/asio.hpp>
#include <boost/noncopyable.hpp>

#include "message/message_handler_invoker.hpp"
#include "connection_handler.hpp"

namespace pgl {
	class server_data;
	struct server_setting;

	class server_thread final : boost::noncopyable {
	public:
		server_thread(boost::asio::ip::tcp::acceptor& acceptor,
			server_data& server_data, const server_setting& server_setting);
		void start();
		void stop();
	private:
		boost::asio::ip::tcp::acceptor& acceptor_;
		server_data& server_data_;
		const server_setting& server_setting_;

		std::unique_ptr<message_handler_invoker> message_handler_invoker_;
		std::vector<std::unique_ptr<connection_handler>> connection_handlers_;
	};
}