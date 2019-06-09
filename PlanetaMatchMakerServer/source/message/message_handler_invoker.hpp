#pragma once

#include <unordered_map>
#include <functional>
#include <cassert>
#include <shared_mutex>

#include "messages.hpp"
#include "message_handler.hpp"
#include "message_handle_parameter.hpp"

namespace pgl {
	// A thread safe container of message handler
	class message_handler_invoker final : boost::noncopyable {
	public:
		using message_handler_generator_type = std::function<std::unique_ptr<message_handler>()>;

		template <message_type MessageType, class MessageHandler>
		void register_handler() {
			assert(handler_generator_map_.find(MessageType) == handler_generator_map_.end());
			std::lock_guard<decltype(mutex_)> lock(mutex_);
			static_assert(std::is_base_of_v<message_handler, MessageHandler>,
				"MessageHandler must be a child class of message_handler.");
			handler_generator_map_.emplace(MessageType, []() { return std::make_unique<MessageHandler>(); });
		}

		void handle_message(message_handle_parameter& param) const;

		void handle_specific_message(message_type specified_message_type, message_handle_parameter& param) const;

	private:
		std::unordered_map<message_type, message_handler_generator_type> handler_generator_map_;
		mutable std::shared_mutex mutex_;

		[[nodiscard]] bool is_handler_exist(const message_type message_type) const {
			std::shared_lock<decltype(mutex_)> lock(mutex_);
			return handler_generator_map_.find(message_type) != handler_generator_map_.end();
		}

		[[nodiscard]] std::unique_ptr<message_handler> make_message_handler(const message_type message_type) const {
			std::shared_lock<decltype(mutex_)> lock(mutex_);
			return handler_generator_map_.at(message_type)();
		}

		void handle_message_impl(bool enable_message_specification, message_type specified_message_type,
		                         message_handle_parameter& param) const;
	};
}
