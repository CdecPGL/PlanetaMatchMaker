#pragma once

#include <type_traits>

#include "messages.hpp"

namespace pgl {
	class message_handler {
	public:
		virtual ~message_handler() = default;
		[[nodiscard]] virtual int get_message_size() const = 0;
		virtual void operator()(const char* data) = 0;
	};

	template <class TMessage>
	class message_handler_base : public message_handler {
		static_assert(std::is_base_of_v<message, TMessage>, "TMessage must be a child class of message.");
	public:
		virtual ~message_handler_base() = default;

		[[nodiscard]] int get_message_size() const override final {
			return sizeof(TMessage);
		}

		void operator()(const char* data) override final {
			decltype(auto) message = reinterpret_cast<const TMessage*>(data);
			handle_message(*message);
		}

	private:
		virtual void handle_message(const TMessage& message) = 0;
	};
}
