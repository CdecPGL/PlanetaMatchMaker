#pragma once

#include <memory>

#include <boost/asio.hpp>

namespace pgl {
	struct server_data;

	struct message_handle_parameter final {
		boost::asio::io_service& io_service;
		boost::asio::ip::tcp::socket& socket;
		boost::asio::streambuf& receive_buff;
		const std::shared_ptr<server_data> server_data;
		boost::asio::yield_context& yield;
		std::chrono::seconds timeout_seconds;
	};
}