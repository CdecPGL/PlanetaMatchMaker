#pragma once

#include <unordered_map>
#include <functional>
#include <cassert>
#include <shared_mutex>

#include "message_handler.hpp"

namespace pgl {
	// A thread safe container of message handler
	class message_handler_container final : boost::noncopyable {
	public:
		template <message_type VMessageType, class TMessageHandler>
		void register_handler() {
			assert(handler_generator_map_.find(VMessageType) == handler_generator_map_.end());
			std::lock_guard<decltype(mutex_)> lock(mutex_);
			static_assert(std::is_base_of_v<message_handler, TMessageHandler>,
				"TMessageHandler must be a child class of message_handler.");
			handler_generator_map_.emplace(VMessageType, []() { return std::make_unique<TMessageHandler>(); });
		}

		[[nodiscard]] bool is_handler_exist(const message_type message_type) const {
			std::shared_lock<decltype(mutex_)> lock(mutex_);
			return handler_generator_map_.find(message_type) != handler_generator_map_.end();
		}

		[[nodiscard]] std::unique_ptr<message_handler> make_message_handler(const message_type message_type) const {
			std::shared_lock<decltype(mutex_)> lock(mutex_);
			return handler_generator_map_.at(message_type)();
		}

	private:
		std::unordered_map<message_type, std::function<std::unique_ptr<message_handler>()>>
		handler_generator_map_;
		mutable std::shared_mutex mutex_;
	};
}
