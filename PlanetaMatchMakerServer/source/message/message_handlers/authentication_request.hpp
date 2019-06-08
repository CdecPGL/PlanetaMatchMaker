#pragma once

#include "../message_handler.hpp"

namespace pgl {
	class authentication_request final : message_handler_base<authentication_request_message> {
		void handle_message(const authentication_request_message& message,
		                    const std::shared_ptr<server_data>& server_data,
		                    boost::asio::yield_context& yield) override;
	};
}
