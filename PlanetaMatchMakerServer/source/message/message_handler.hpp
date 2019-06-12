#pragma once

#include <boost/asio/spawn.hpp>

#include "async/timer.hpp"
#include "async/read_write.hpp"
#include "server/server_error.hpp"

#include "message_handle_parameter.hpp"

namespace pgl {
	class message_handler {
	public:
		message_handler() = default;
		message_handler(const message_handler& message_handler) = delete;
		message_handler(message_handler&& message_handler) = delete;
		virtual ~message_handler() = default;
		message_handler& operator=(const message_handler& message_handler) = delete;
		message_handler& operator=(message_handler&& message_handler) = delete;
		virtual void operator()(const char* data, std::shared_ptr<message_handle_parameter> param) = 0;
		[[nodiscard]] virtual int get_message_size() const = 0;
	protected:
		// Send data to remote endpoint. server_error will be thrown when send error occured.
		template <typename FirstData, typename... RestData>
		static void send(std::shared_ptr<message_handle_parameter> param, FirstData&& first_data,
		                 RestData&&... rest_data) {
			try {
				if constexpr (sizeof...(RestData) > 0) {
					execute_timed_async_operation(param->io_service, param->socket, param->timeout_seconds,
					                              [param, first_data, rest_data...]()
					                              {
						                              packed_async_write(
							                              param->socket, param->yield, first_data, rest_data...);
					                              });
				} else {
					execute_timed_async_operation(param->io_service, param->socket, param->timeout_seconds,
					                              [param, first_data]()
					                              {
						                              async_write(
							                              param->socket,
							                              boost::asio::buffer(&first_data, sizeof(first_data)),
							                              param->yield);
					                              });
				}
			} catch (const boost::system::system_error& e) {
				throw server_error(server_error_code::message_send_error, e.code().message());
			}
		}
	};

	template <class Message>
	class message_handler_base : public message_handler {
	public:
		message_handler_base() = default;
		message_handler_base(const message_handler_base& message_handler_base) = delete;
		message_handler_base(message_handler_base&& message_handler_base) = delete;
		virtual ~message_handler_base() = default;
		message_handler_base& operator=(const message_handler_base& message_handler_base) = delete;
		message_handler_base& operator=(message_handler_base&& message_handler_base) = delete;

		[[nodiscard]] int get_message_size() const override final {
			return sizeof(Message);
		}

		void operator()(const char* data, std::shared_ptr<message_handle_parameter> param) override final {
			decltype(auto) message = reinterpret_cast<const Message*>(data);
			handle_message(*message, std::move(param));
		}

	private:
		virtual void handle_message(const Message& message, std::shared_ptr<message_handle_parameter> param) = 0;
	};
}
