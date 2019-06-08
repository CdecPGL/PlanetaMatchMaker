#pragma once

#include "../message_handler.hpp"

namespace pgl {
	class authentication_request_message_handler final : public message_handler_base<authentication_request_message> {
		void handle_message(const authentication_request_message& message, message_handle_parameter& param) override;
	};
}
