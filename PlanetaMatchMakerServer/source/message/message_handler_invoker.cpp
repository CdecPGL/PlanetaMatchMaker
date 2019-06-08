﻿#include "message_handler_invoker.hpp"

#include <boost/asio.hpp>
#include <boost/asio/spawn.hpp>

#include "nameof.hpp"

#include "async/timer.hpp"
#include "utilities/io_utility.hpp"
#include "message_handle_error.hpp"

using namespace std;
using namespace boost;

namespace pgl {
	void message_handler_invoker::
	handle_message(message_handle_parameter& param) const {
		handle_message_impl(false, {}, param);
	}

	auto message_handler_invoker::handle_specific_message(const message_type specified_message_type,
	                                                      message_handle_parameter& param) const -> void {
		handle_message_impl(true, specified_message_type, param);
	}

	void message_handler_invoker::handle_message_impl(const bool enable_message_specification,
	                                                  message_type specified_message_type,
	                                                  message_handle_parameter& param) const {

		// Receive a message header
		try {
			execute_timed_async_operation(param.io_service, param.socket, param.timeout_seconds, [&]()
			{
				async_read(param.socket, param.receive_buff, asio::transfer_exactly(sizeof(message_header)),
				           param.yield);
			});
		} catch (const system::system_error& e) {
			if (e.code() == asio::error::operation_aborted) {
				throw message_handle_error(message_handle_error_code::message_reception_timeout);
			}

			if (e.code() && e.code() != asio::error::eof) {
				throw message_handle_error(message_handle_error_code::message_header_reception_error);
			}
		}

		// Analyze received message header
		auto header = asio::buffer_cast<const message_header*>(param.receive_buff.data());
		param.receive_buff.consume(sizeof(message_header));
		if (!is_handler_exist(header->message_type)) {
			throw message_handle_error(message_handle_error_code::invalid_message_type,
			                           generate_string(static_cast<int>(header->message_type)));
		}

		if (enable_message_specification && header->message_type != specified_message_type) {
			throw message_handle_error(message_handle_error_code::message_type_mismatch,
			                           generate_string("expected: ", NAMEOF_ENUM(specified_message_type), ", actual: ",
			                                           NAMEOF_ENUM(header->message_type)));
		}

		const auto message_handler = make_message_handler(header->message_type);
		const auto message_size = message_handler->get_message_size();
		print_line("Message header received. (type: ", NAMEOF_ENUM(header->message_type), "size: ", message_size);

		// Receive a body of message
		try {
			execute_timed_async_operation(param.io_service, param.socket, param.timeout_seconds, [&]()
			{
				async_read(param.socket, param.receive_buff,
				           asio::transfer_exactly(message_size), param.yield);
			});
		} catch (const system::system_error& e) {
			if (e.code() == asio::error::operation_aborted) {
				throw message_handle_error(message_handle_error_code::message_reception_timeout);
			}

			if (e.code() && e.code() != asio::error::eof) {
				throw message_handle_error(message_handle_error_code::message_body_reception_error);
			}
		}

		// process received message
		const auto* data = asio::buffer_cast<const char*>(param.receive_buff.data());
		(*message_handler)(data, param);
		param.receive_buff.consume(param.receive_buff.size());
		print_line("Message processed. (type: ", NAMEOF_ENUM(header->message_type), "size: ", message_size);
	}
}
