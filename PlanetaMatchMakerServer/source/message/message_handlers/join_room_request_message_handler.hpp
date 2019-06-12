#pragma once

#include "../messages.hpp"
#include "../message_handler.hpp"

namespace pgl {
	class join_room_request_message_handler final : public message_handler_base<join_room_request_message> {
		void handle_message(const join_room_request_message& message,
		                    std::shared_ptr<message_handle_parameter> param) override;
	};
}
