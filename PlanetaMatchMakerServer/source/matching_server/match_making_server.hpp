#pragma once

#include <cstdint>

#include <boost/asio.hpp>
#include <boost/noncopyable.hpp>

#include "network/transport_layer.hpp"

namespace pgl {
	class message_handler_container;

	class match_making_server final : boost::noncopyable {
	public:
		match_making_server(std::shared_ptr<message_handler_container> message_handler_container,
		                    boost::asio::io_service& io_service, const ip_version ip_version,
		                    const std::uint16_t port_number, const std::uint32_t time_out_seconds);

		void start();
	private:
		std::shared_ptr<message_handler_container> message_handler_container_;
		boost::asio::io_service& io_service_;
		boost::asio::ip::tcp::acceptor acceptor_;
		boost::asio::ip::tcp::socket socket_;
		boost::asio::streambuf receive_buff_;
		boost::asio::steady_timer timer_;
		std::uint32_t time_out_seconds_;
		bool is_canceled_ = false;

		void start_timer();
		void cancel_timer();
		void on_timer(const boost::system::error_code& error);
	};
}
