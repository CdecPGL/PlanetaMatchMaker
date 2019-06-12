#pragma once

#include "../messages.hpp"
#include "../message_handler.hpp"

namespace pgl {
	class list_room_group_request_message_handler final : public message_handler_base<list_room_group_request_message> {
		void handle_message(const list_room_group_request_message& message,
		                    std::shared_ptr<message_handle_parameter> param) override;
	};
}
