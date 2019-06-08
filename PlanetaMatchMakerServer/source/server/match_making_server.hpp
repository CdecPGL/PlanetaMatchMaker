#pragma once

#include <cstdint>
#include <memory>

#include <boost/asio.hpp>
#include <boost/noncopyable.hpp>

#include "network/network_layer.hpp"

namespace pgl {
	class message_handler_invoker;
	struct server_data;

	class match_making_server final : boost::noncopyable {
	public:
		match_making_server(std::shared_ptr<server_data> server_data,
		                    std::shared_ptr<message_handler_invoker> message_handler_container,
		                    boost::asio::io_service& io_service, ip_version ip_version,
		                    std::uint16_t port_number, std::uint32_t time_out_seconds);

		void start();
	private:
		std::shared_ptr<server_data> server_data_;
		std::shared_ptr<message_handler_invoker> message_handler_container_;
		boost::asio::io_service& io_service_;
		boost::asio::ip::tcp::acceptor acceptor_;
		boost::asio::ip::tcp::socket socket_;
		boost::asio::streambuf receive_buff_;
		boost::asio::steady_timer timer_;
		std::uint32_t time_out_seconds_;

		void restart();
	};
}
