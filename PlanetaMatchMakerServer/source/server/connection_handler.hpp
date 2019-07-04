#pragma once

#include <memory>

#include <boost/asio.hpp>
#include <boost/noncopyable.hpp>

#include "session/session_data.hpp"

namespace pgl {
	class message_handler_invoker;
	class server_data;
	struct server_setting;
	class session_data;

	class connection_handler final : boost::noncopyable {
	public:
		connection_handler(boost::asio::ip::tcp::acceptor& acceptor,
			server_data& server_data, const server_setting& server_setting,
			message_handler_invoker& message_handler_invoker);
		void start();
		void stop();
	private:
		boost::asio::ip::tcp::acceptor& acceptor_;
		server_data& server_data_;
		const server_setting& server_setting_;
		message_handler_invoker& message_handler_invoker_;

		boost::asio::ip::tcp::socket socket_;
		std::unique_ptr<session_data> session_data_;

		void finalize() const;
		void restart();
	};
}
