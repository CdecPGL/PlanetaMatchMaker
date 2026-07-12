#pragma once

#include <memory>
#include <mutex>
#include <vector>

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>

#include "message/message_handler_invoker.hpp"

namespace pgl {
	class server_data;
	class server_session;
	class server_tls_context;
	struct server_setting;

	class server_thread final : boost::noncopyable {
	public:
		server_thread(boost::asio::ip::tcp::acceptor& acceptor,
			std::mutex& acceptor_mutex, server_tls_context& tls_context, server_data& server_data,
			const server_setting& server_setting);
		void start();
		void stop();
	private:
		boost::asio::ip::tcp::acceptor& acceptor_;
		std::mutex& acceptor_mutex_;
		server_tls_context& tls_context_;
		server_data& server_data_;
		const server_setting& server_setting_;

		std::shared_ptr<const message_handler_invoker> message_handler_invoker_;
		std::vector<std::shared_ptr<server_session>> server_sessions_;
	};
}
