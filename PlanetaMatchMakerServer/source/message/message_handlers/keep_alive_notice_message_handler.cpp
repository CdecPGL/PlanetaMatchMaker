#include "keep_alive_notice_message_handler.hpp"

namespace pgl {
	keep_alive_notice_message_handler::handle_return_t keep_alive_notice_message_handler::handle_message(
		const keep_alive_notice_message& message [[maybe_unused]],
		std::shared_ptr<message_handle_parameter> param) { return {{}, false}; }
}
