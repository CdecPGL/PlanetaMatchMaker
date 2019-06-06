#pragma once

#include "../message_handler.hpp"

namespace pgl {
	class list_room_request_message_handler final : public message_handler_base<list_room_request_message> {
	public:
	private:
		void handle_message(const list_room_request_message& message) override;
	};
}
