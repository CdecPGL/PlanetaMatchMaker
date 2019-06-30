#pragma once

#include <boost/asio/spawn.hpp>

#include "message_handle_utilities.hpp"
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
		virtual void operator()(std::shared_ptr<message_handle_parameter> param) = 0;
		[[nodiscard]] virtual int get_message_size() const = 0;
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

		void operator()(std::shared_ptr<message_handle_parameter> param) override final {
			Message message;
			try {
				receive(param, message);
			} catch (const server_error& e) {
				throw server_error(server_error_code::message_body_reception_error, e.message());
			}
			handle_message(message, std::move(param));
		}

	private:
		virtual void handle_message(const Message& message, std::shared_ptr<message_handle_parameter> param) = 0;
	};
}
