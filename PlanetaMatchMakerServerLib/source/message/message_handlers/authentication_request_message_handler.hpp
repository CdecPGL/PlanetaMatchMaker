#pragma once

#include "../messages.hpp"
#include "../message_handler.hpp"

namespace pgl {
	class authentication_request_message_handler final : public message_handler_base<authentication_request_message,
			authentication_reply_message> {
		message_attachment_policy get_message_attachment_policy(const authentication_request_message& message,
			const std::shared_ptr<message_handle_parameter>& param) const override;
		std::optional<handle_return_t> validate_message_before_attachment(
			const authentication_request_message& message, message_attachment_size_t attachment_size,
			const std::shared_ptr<message_handle_parameter>& param) override;
		handle_return_t handle_message_attachment_error(const authentication_request_message& message,
			message_attachment_error error, message_attachment_size_t attachment_size,
			const std::shared_ptr<message_handle_parameter>& param) override;
		handle_return_t handle_message(const authentication_request_message& message,
			const std::vector<uint8_t>& credential,
			std::shared_ptr<message_handle_parameter> param) override;
	};
}
