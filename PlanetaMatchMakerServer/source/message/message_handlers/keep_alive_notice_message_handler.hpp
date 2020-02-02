#pragma once

#include "../messages.hpp"
#include "../message_handler.hpp"

namespace pgl {
	class keep_alive_notice_message_handler
		final : public message_handler_base<keep_alive_notice_message> {
	public:
		void handle_message(const keep_alive_notice_message& message,
			std::shared_ptr<message_handle_parameter> param) override;
	};
}
