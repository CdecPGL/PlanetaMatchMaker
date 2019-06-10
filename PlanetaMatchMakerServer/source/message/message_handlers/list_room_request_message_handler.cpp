#include <iostream>

#include "nameof.hpp"

#include "list_room_request_message_handler.hpp"

using namespace std;

namespace pgl {
	void list_room_request_message_handler::handle_message(const list_room_request_message& message,
	                                                       message_handle_parameter& param) {
		reply_message_header header{
			message_type::list_room_reply,
			message_error_code::ok
		};

		list_room_reply reply{

		};
	}
}
