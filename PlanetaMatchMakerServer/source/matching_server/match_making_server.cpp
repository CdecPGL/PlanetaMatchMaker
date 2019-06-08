#include <iostream>

#include <boost/bind.hpp>
#include <boost/asio/spawn.hpp>
#include <utility>

#include "nameof.hpp"

#include "message/message_handler_invoker.hpp"
#include "match_making_server.hpp"

using namespace std;
using namespace boost;

namespace pgl {
	constexpr int buffer_size = 256;

	match_making_server::
	match_making_server(std::shared_ptr<server_data> server_data,
	                    std::shared_ptr<message_handler_invoker> message_handler_container,
	                    asio::io_service& io_service, const ip_version ip_version,
	                    const std::uint16_t port_number,
	                    const std::uint32_t time_out_seconds)
		: server_data_(std::move(server_data)),
		  message_handler_container_(std::move(message_handler_container)),
		  io_service_(io_service),
		  acceptor_(io_service, asio::ip::tcp::endpoint(get_tcp(ip_version), port_number)),
		  socket_(io_service),
		  receive_buff_(buffer_size),
		  timer_(io_service),
		  time_out_seconds_(time_out_seconds) {
		cout << "Server instance is generated with ip version " << NAMEOF_ENUM(ip_version) << " and port number " <<
			port_number << "." << endl;
	}

	void match_making_server::start() {
		spawn(io_service_, [&](asio::yield_context yield)
		{
			cout << "Start to accept." << endl;
			system::error_code error;
			acceptor_.async_accept(socket_, yield[error]);
			if (error) {
				cerr << "Accept failed: " << error.message() << endl;
				start();
				return;
			}

			cout << "Accepted new connection. Start to receive message." << endl;

			// Authenticate client
			auto message_handler_param = message_handle_parameter{
				io_service_, socket_, receive_buff_, server_data_, yield
			};
			message_handler_container_->handle_specific_message(message_type::authentication_request,
			                                                    message_handler_param);

			// Receive message
			while (true) {
				message_handler_container_->handle_message(message_handler_param);
			}
		});
	}

	void match_making_server::start_timer() {
		is_canceled_ = false;
		timer_.expires_from_now(chrono::seconds(time_out_seconds_));
		timer_.async_wait(boost::bind(&match_making_server::on_timer, this, _1));
	}

	void match_making_server::cancel_timer() {
		timer_.cancel();
		is_canceled_ = true;
	}

	void match_making_server::on_timer(const system::error_code& error) {
		if (!error && !is_canceled_) {
			socket_.cancel(); // 通信処理をキャンセルする。受信ハンドラがエラーになる
		}
	}
}
