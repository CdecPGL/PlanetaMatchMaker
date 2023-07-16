#pragma once

#include <memory>

#include <boost/asio.hpp>

#include "session/session_data.hpp"

namespace pgl {
	class message_handler_invoker;
	class server_data;
	struct server_setting;
	class session_data;

	class server_session final : public std::enable_shared_from_this<server_session>, boost::noncopyable {
	public:
		server_session(boost::asio::ip::tcp::acceptor& acceptor,
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
		void remove_hosting_room_if_need()const;
		void remove_player_full_name_if_need()const;
	};
}
