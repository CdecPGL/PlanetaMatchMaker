#pragma once

#include <memory>

#include <boost/asio.hpp>
#include <boost/asio/spawn.hpp>

namespace pgl {
	class server_data;
	class session_data;

	struct message_handle_parameter final {
		boost::asio::io_service& io_service;
		boost::asio::ip::tcp::socket& socket;
		const std::shared_ptr<server_data> server_data;
		boost::asio::yield_context& yield;
		std::chrono::seconds timeout_seconds;
		session_data& session_data;
	};
}
