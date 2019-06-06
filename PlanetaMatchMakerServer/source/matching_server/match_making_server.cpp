#include <iostream>

#include <boost/bind.hpp>
#include <boost/asio/spawn.hpp>
#include <utility>

#include "nameof.hpp"

#include "message/message_handler_container.hpp"
#include "match_making_server.hpp"

using namespace std;
using namespace boost;

namespace pgl {
	constexpr int buffer_size = 128;

	match_making_server::
	match_making_server(std::shared_ptr<message_handler_container> message_handler_container,
	                    asio::io_service& io_service, const ip_version ip_version,
	                    const std::uint16_t port_number,
	                    const std::uint32_t time_out_seconds) : message_handler_container_(
		                                                            std::move(message_handler_container)),
	                                                            io_service_(io_service),
	                                                            acceptor_(io_service,
	                                                                      asio::ip::tcp::endpoint(get_tcp(ip_version),
	                                                                                              port_number)),
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
			cout << "Start server." << endl;
			system::error_code error;
			acceptor_.async_accept(socket_, yield[error]);
			if (error) {
				cerr << "accept failed: " << error.message() << endl;
				return;
			}

			cout << "accept correct!" << endl;
			cout << "Start receive" << endl;

			//メッセージヘッダーの受信
			start_timer();
			async_read(socket_, receive_buff_, asio::transfer_exactly(sizeof(message_header)), yield[error]);
			if (error == asio::error::operation_aborted) {
				std::cerr << "time out" << std::endl;
				return;
			}

			if (error && error != asio::error::eof) {
				cerr << "failed to receive message header: " << error.message() << endl;
				return;
			}

			cancel_timer();

			//メッセージヘッダーの解析
			auto header = asio::buffer_cast<const message_header*>(receive_buff_.data());
			if (!message_handler_container_->is_handler_exist(header->message_type)) {
				cerr << "Invalid message type received: " << static_cast<int>(header->message_type) << endl;;
				return;
			}

			const auto message_handler = message_handler_container_->make_message_handler(header->message_type);
			const auto message_size = message_handler->get_message_size();
			cout << "message type: " << NAMEOF_ENUM(header->message_type) << endl;
			cout << "message size: " << message_size << endl;

			//メッセージ本体の受信
			start_timer();
			async_read(socket_, receive_buff_, asio::transfer_exactly(message_size), yield[error]);
			if (error == asio::error::operation_aborted) {
				std::cerr << "time out" << std::endl;
				return;
			}

			if (error && error != asio::error::eof) {
				cerr << "failed to receive message: " << error.message() << endl;
				return;
			}

			cancel_timer();

			//メッセージ本体の処理
			const auto* data = asio::buffer_cast<const char*>(receive_buff_.data());
			(*message_handler)(data, yield);
			receive_buff_.consume(receive_buff_.size());
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

	void match_making_server::on_timer(const boost::system::error_code& error) {
		if (!error && !is_canceled_) {
			socket_.cancel(); // 通信処理をキャンセルする。受信ハンドラがエラーになる
		}
	}
}
