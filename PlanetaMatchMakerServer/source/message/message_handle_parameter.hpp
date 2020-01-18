#pragma once

#include <boost/asio/spawn.hpp>

namespace pgl {
	class server_data;
	class session_data;
	struct server_setting;

	struct message_handle_parameter final {
		boost::asio::ip::tcp::socket& socket;
		server_data& server_data;
		boost::asio::yield_context yield;
		std::chrono::seconds timeout_seconds;
		session_data& session_data;
		const server_setting& server_setting;
	};
}
