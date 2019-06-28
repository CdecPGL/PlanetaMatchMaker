#pragma once

#include "../messages.hpp"
#include "../message_handler.hpp"

namespace pgl {
	class update_room_status_notice_message_handler
		final : public message_handler_base<update_room_status_notice_message> {
	public:
		void handle_message(const update_room_status_notice_message& message,
			std::shared_ptr<message_handle_parameter> param) override;
	};
}
