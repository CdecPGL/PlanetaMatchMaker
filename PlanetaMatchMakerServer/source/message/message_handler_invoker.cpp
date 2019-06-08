#include "message_handler_container.hpp"

#include <iostream>

#include <boost/asio.hpp>
#include <boost/asio/spawn.hpp>

#include "nameof.hpp"

using namespace std;
using namespace boost;

namespace pgl {
	void message_handler_invoker::handle_message(asio::ip::tcp::socket& socket,
	                                               asio::streambuf& receive_buff,
	                                               const std::shared_ptr<server_data>& server_data,
	                                               asio::yield_context& yield) const {
		handle_specific_message_impl(false, {}, socket, receive_buff, server_data, yield);
	}

	auto message_handler_invoker::handle_specific_message(const message_type specified_message_type,
	                                                        asio::ip::tcp::socket& socket,
	                                                        asio::streambuf& receive_buff,
	                                                        const std::shared_ptr<server_data>& server_data,
	                                                        asio::yield_context& yield) const -> void {
		handle_specific_message_impl(true, specified_message_type, socket, receive_buff, server_data, yield);
	}

	void message_handler_invoker::handle_specific_message_impl(const bool enable_message_specification,
	                                                             message_type specified_message_type,
	                                                             asio::ip::tcp::socket& socket,
	                                                             asio::streambuf& receive_buff,
	                                                             const std::shared_ptr<server_data>& server_data,
	                                                             asio::yield_context& yield) const {
		system::error_code error;

		// Receive a message header
		//start_timer();
		async_read(socket, receive_buff, asio::transfer_exactly(sizeof(message_header)), yield[error]);
		if (error == asio::error::operation_aborted) {
			std::cerr << "time out" << std::endl;
			return;
		}

		if (error && error != asio::error::eof) {
			cerr << "failed to receive message header: " << error.message() << endl;
			return;
		}

		//cancel_timer();

		// Analyze received message header
		auto header = asio::buffer_cast<const message_header*>(receive_buff.data());
		receive_buff.consume(sizeof(message_header));
		if (!is_handler_exist(header->message_type)) {
			cerr << "Invalid message type received: " << static_cast<int>(header->message_type) << endl;
			return;
		}

		if (enable_message_specification && header->message_type != specified_message_type) {
			cerr << NAMEOF_ENUM(specified_message_type) << " is expected, but " << NAMEOF_ENUM(header->message_type) <<
				" is received." << endl;
			return;
		}

		const auto message_handler = make_message_handler(header->message_type);
		const auto message_size = message_handler->get_message_size();
		cout << "message type: " << NAMEOF_ENUM(header->message_type) << endl;
		cout << "message size: " << message_size << endl;

		// Receive a body of message
		//start_timer();
		async_read(socket, receive_buff, asio::transfer_exactly(message_size), yield[error]);
		if (error == asio::error::operation_aborted) {
			std::cerr << "time out" << std::endl;
			return;
		}

		if (error && error != asio::error::eof) {
			cerr << "failed to receive message: " << error.message() << endl;
			return;
		}

		//cancel_timer();

		// process received message
		const auto* data = asio::buffer_cast<const char*>(receive_buff.data());
		(*message_handler)(data, server_data, yield);
		receive_buff.consume(receive_buff.size());
	}
}
