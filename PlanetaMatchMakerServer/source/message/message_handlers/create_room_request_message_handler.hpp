#pragma once

#include "../messages.hpp"
#include "../message_handler.hpp"

namespace pgl {
	class create_room_request_message_handler final : public message_handler_base<create_room_request_message> {
		void handle_message(const create_room_request_message& message,
		                    std::shared_ptr<message_handle_parameter> param) override;
	};
}
