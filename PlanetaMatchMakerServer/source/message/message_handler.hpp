#pragma once

#include <boost/asio/spawn.hpp>

#include "minimal_serializer/serializer.hpp"
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
		[[nodiscard]] virtual size_t get_message_size() const = 0;
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

		[[nodiscard]] size_t get_message_size() const override final {
			return minimal_serializer::serialized_size_v<Message>;
		}

		void operator()(std::shared_ptr<message_handle_parameter> param) override final {
			Message message;
			receive(param, message);
			handle_message(message, std::move(param));
		}

	private:
		virtual void handle_message(const Message& message, std::shared_ptr<message_handle_parameter> param) = 0;
	};
}
