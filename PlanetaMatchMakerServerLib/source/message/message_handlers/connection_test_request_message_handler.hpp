#pragma once

#include "../messages.hpp"
#include "../message_handler.hpp"

namespace pgl {
	class connection_test_request_message_handler final : public message_handler_base<connection_test_request_message,
			connection_test_reply_message> {
		handle_return_t handle_message(const connection_test_request_message& message,
			std::shared_ptr<message_handle_parameter> param) override;
	};
}
