#include "message_handler_container_factory.hpp"
#include "message_handler_container.hpp"
#include "message_handler/list_room_request_message_handler.hpp"

namespace pgl {
	std::shared_ptr<message_handler_container> message_handler_container_factory::
	make_standard_message_handler_container() {
		auto container = std::make_shared<message_handler_container>();
		container->register_handler<message_type::list_room_request, list_room_request_message_handler>();
		return container;
	}
}
