#include "message_handler_invoker_factory.hpp"
#include "message_handler_invoker.hpp"
#include "message_handlers/authentication_request_message_handler.hpp"
#include "message_handlers/list_room_group_request_message_handler.hpp"
#include "message_handlers/create_room_request_message_handler.hpp"
#include "message_handlers/join_room_request_message_handler.hpp"
#include "message_handlers/list_room_request_message_handler.hpp"

namespace pgl {
	std::shared_ptr<message_handler_invoker> message_handler_invoker_factory::make_standard() {
		auto invoker = std::make_shared<message_handler_invoker>();
		invoker->register_handler<message_type::authentication_request, authentication_request_message_handler>();
		invoker->register_handler<message_type::list_room_group_request, list_room_group_request_message_handler>();
		invoker->register_handler<message_type::create_room_request, create_room_request_message_handler>();
		invoker->register_handler<message_type::join_room_request, join_room_request_message_handler>();
		invoker->register_handler<message_type::list_room_request, list_room_request_message_handler>();
		return invoker;
	}
}
