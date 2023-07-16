#include "message_handler_invoker.hpp"

#include <boost/asio.hpp>

#include "logger/log.hpp"
#include "server/server_errors.hpp"
#include "message_handle_utilities.hpp"

using namespace std;
using namespace boost;
using namespace minimal_serializer;

namespace pgl {
	void message_handler_invoker::
	handle_message(std::shared_ptr<message_handle_parameter> param) const {
		handle_message_impl(false, {}, std::move(param));
	}

	void message_handler_invoker::handle_specific_message(const message_type specified_message_type,
		std::shared_ptr<message_handle_parameter> param) const {
		handle_message_impl(true, specified_message_type, std::move(param));
	}

	void message_handler_invoker::handle_message_impl(const bool enable_message_specification,
		message_type specified_message_type, const std::shared_ptr<message_handle_parameter> param) const {
		// Receive ana analyze a message header
		request_message_header header{};
		receive(param, header);

		if (!is_handler_exist(header.message_type)) {
			const auto error_message = generate_string("Invalid message type: ", static_cast<int>(header.message_type));
			throw server_session_intended_disconnect_error(error_message);
		}

		if (enable_message_specification && header.message_type != specified_message_type) {
			const auto error_message = generate_string("Unexpected message type. expected: ", specified_message_type,
				", actual: ", header.message_type);
			throw server_session_intended_disconnect_error(error_message);
		}

		const auto message_handler = make_message_handler(header.message_type);
		constexpr auto header_size = minimal_serializer::serialized_size_v<request_message_header>;
		const auto message_size = message_handler->get_message_size();
		log_with_endpoint(log_level::info, param->socket.remote_endpoint(), "Message header received. (type: ",
			header.message_type, ", size: ", header_size, ")");

		// Receive and process a body of message
		(*message_handler)(header, param);

		log_with_endpoint(log_level::info, param->socket.remote_endpoint(), "Message processed. (type: ",
			header.message_type, ", size: ", message_size, ")");
	}
}
