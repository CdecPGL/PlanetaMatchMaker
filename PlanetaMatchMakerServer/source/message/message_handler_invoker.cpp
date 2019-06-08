#include "message_handler_invoker.hpp"

#include <iostream>

#include <boost/asio.hpp>
#include <boost/asio/spawn.hpp>

#include "nameof.hpp"

using namespace std;
using namespace boost;

namespace pgl {
	void message_handler_invoker::
	handle_message(message_handle_parameter& param) const {
		handle_specific_message_impl(false, {}, param);
	}

	auto message_handler_invoker::handle_specific_message(const message_type specified_message_type,
	                                                      message_handle_parameter& param) const -> void {
		handle_specific_message_impl(true, specified_message_type, param);
	}

	void message_handler_invoker::handle_specific_message_impl(const bool enable_message_specification,
	                                                           message_type specified_message_type,
	                                                           message_handle_parameter& param) const {
		system::error_code error;

		// Receive a message header

		//start_timer();
		async_read(param.socket, param.receive_buff, asio::transfer_exactly(sizeof(message_header)), param.yield[error]);
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
		auto header = asio::buffer_cast<const message_header*>(param.receive_buff.data());
		param.receive_buff.consume(sizeof(message_header));
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
		async_read(param.socket, param.receive_buff, asio::transfer_exactly(message_size), param.yield[error]);
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
		const auto* data = asio::buffer_cast<const char*>(param.receive_buff.data());
		(*message_handler)(data, param);
		param.receive_buff.consume(param.receive_buff.size());
	}
}
