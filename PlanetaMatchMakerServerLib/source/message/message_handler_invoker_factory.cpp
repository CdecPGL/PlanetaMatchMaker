#include "message_handler_invoker_factory.hpp"
#include "message_handler_invoker.hpp"
#include "message_handlers/authentication_request_message_handler.hpp"
#include "message_handlers/create_room_request_message_handler.hpp"
#include "message_handlers/join_room_request_message_handler.hpp"
#include "message_handlers/list_room_request_message_handler.hpp"
#include "message_handlers/update_room_status_notice_message_handler.hpp"
#include "message_handlers/connection_test_request_message_handler.hpp"
#include "message_handlers/keep_alive_notice_message_handler.hpp"

namespace pgl {
	void register_handlers(message_handler_invoker& invoker) {
		invoker.register_handler<message_type::authentication, authentication_request_message_handler>();
		invoker.register_handler<message_type::create_room, create_room_request_message_handler>();
		invoker.register_handler<message_type::join_room, join_room_request_message_handler>();
		invoker.register_handler<message_type::list_room, list_room_request_message_handler>();
		invoker.register_handler<message_type::update_room_status, update_room_status_notice_message_handler
		>();
		invoker.register_handler<message_type::connection_test, connection_test_request_message_handler>();
		invoker.register_handler<message_type::keep_alive, keep_alive_notice_message_handler>();
	}

	std::shared_ptr<message_handler_invoker> message_handler_invoker_factory::make_shared_standard() {
		auto invoker = std::make_shared<message_handler_invoker>();
		register_handlers(*invoker);
		log(log_level::info, "Generate standard message handler invoker.");
		return invoker;
	}

	std::unique_ptr<message_handler_invoker> message_handler_invoker_factory::make_unique_standard() {
		auto invoker = std::make_unique<message_handler_invoker>();
		register_handlers(*invoker);
		log(log_level::info, "Generate standard message handler invoker.");
		return invoker;
	}
}
