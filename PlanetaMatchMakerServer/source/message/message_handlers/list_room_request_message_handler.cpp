#include <iostream>

#include "nameof.hpp"

#include "list_room_request_message_handler.hpp"

using namespace std;

namespace pgl {
	void list_room_request_message_handler::handle_message(const list_room_request_message& message,
	                                                       const std::shared_ptr<server_data>& server_data,
	                                                       boost::asio::yield_context& yield) {
		cout << NAMEOF_VAR_TYPE(this) << endl;
	}
}
