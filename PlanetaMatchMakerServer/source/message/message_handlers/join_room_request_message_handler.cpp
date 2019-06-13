#include "join_room_request_message_handler.hpp"

namespace pgl {
	void join_room_request_message_handler::handle_message(const join_room_request_message& message,
	                                                       std::shared_ptr<message_handle_parameter> param) {
		join_room_reply_message reply{};
		check_remote_endpoint_authority<message_type::join_room_reply>(param, reply);
	}
}
