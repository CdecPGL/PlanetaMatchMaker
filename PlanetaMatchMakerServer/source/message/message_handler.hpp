#pragma once

#include <type_traits>

#include <boost/asio/spawn.hpp>

#include "messages.hpp"
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
		virtual void operator()(const char* data, message_handle_parameter& param) = 0;
		[[nodiscard]] virtual int get_message_size() const = 0;
	};

	template <class TMessage>
	class message_handler_base : public message_handler {
		static_assert(std::is_base_of_v<message, TMessage>, "TMessage must be a child class of message.");
	public:
		message_handler_base() = default;
		message_handler_base(const message_handler_base& message_handler_base) = delete;
		message_handler_base(message_handler_base&& message_handler_base) = delete;
		virtual ~message_handler_base() = default;
		message_handler_base& operator=(const message_handler_base& message_handler_base) = delete;
		message_handler_base& operator=(message_handler_base&& message_handler_base) = delete;

		[[nodiscard]] int get_message_size() const override final {
			return sizeof(TMessage);
		}

		void operator()(const char* data, message_handle_parameter& param) override final {
			decltype(auto) message = reinterpret_cast<const TMessage*>(data);
			handle_message(*message, param);
		}

	private:
		virtual void handle_message(const TMessage& message, message_handle_parameter& param) = 0;
	};
}
