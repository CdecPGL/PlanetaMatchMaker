#pragma once

#include <unordered_map>
#include <functional>
#include <cassert>

#include "logger/log.hpp"
#include "messages.hpp"
#include "message_handler.hpp"
#include "message_handle_parameter.hpp"

namespace pgl {
	// A container of message handler. This is not thread safe
	class message_handler_invoker final : boost::noncopyable {
	public:
		using message_handler_generator_type = std::function<std::unique_ptr<message_handler>()>;

		template <message_type MessageType, class MessageHandler> requires(std::derived_from<MessageHandler, message_handler>)
		void register_handler() {
			assert(!handler_generator_map_.contains(MessageType));
			log(log_level::debug, "Register message handler (", NAMEOF_TYPE(MessageHandler), ") for ", MessageType,
				".");
			handler_generator_map_.emplace(MessageType, []() {
				return std::make_unique<MessageHandler>();
			});
		}

		void handle_message(std::shared_ptr<message_handle_parameter> param, bool check_session_key) const;

		void handle_specific_message(message_type specified_message_type,
			std::shared_ptr<message_handle_parameter> param, bool check_session_key) const;

	private:
		std::unordered_map<message_type, message_handler_generator_type> handler_generator_map_;

		[[nodiscard]] bool is_handler_exist(const message_type message_type) const {
			return handler_generator_map_.contains(message_type);
		}

		[[nodiscard]] std::unique_ptr<message_handler> make_message_handler(const message_type message_type) const {
			return handler_generator_map_.at(message_type)();
		}

		void handle_message_impl(bool enable_message_specification, message_type specified_message_type,
			std::shared_ptr<message_handle_parameter> param, bool check_session_key) const;
	};
}
