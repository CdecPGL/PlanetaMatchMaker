#include "message_handler_invoker.hpp"

#include <boost/asio.hpp>

#include "utilities/log.hpp"
#include "server/server_error.hpp"
#include "session/session_data.hpp"
#include "message_handle_utilities.hpp"

using namespace std;
using namespace boost;

namespace pgl {
	void message_handler_invoker::
	handle_message(std::shared_ptr<message_handle_parameter> param, const bool check_session_key) const {
		handle_message_impl(false, {}, std::move(param), check_session_key);
	}

	auto message_handler_invoker::handle_specific_message(const message_type specified_message_type,
		std::shared_ptr<message_handle_parameter> param, const bool check_session_key) const ->
	void {
		handle_message_impl(true, specified_message_type, std::move(param), check_session_key);
	}

	void message_handler_invoker::handle_message_impl(const bool enable_message_specification,
		message_type specified_message_type, std::shared_ptr<message_handle_parameter> param,
		const bool check_session_key) const {

		// Receive ana analyze a message header
		request_message_header header{};
		receive(param, header);

		// Check if a session is valid if need
		if (check_session_key) {
			if (!param->session_data.is_session_key_generated()) {
				throw server_error(false, server_error_code::invalid_session, "A session key is not generated.");
			}
			if (!param->session_data.check_session_key(header.session_key)) {
				throw server_error(false, server_error_code::invalid_session,
					minimal_serializer::generate_string("A session key(", header.session_key, ") is not valid."));
			}
		}

		if (!is_handler_exist(header.message_type)) {
			throw server_error(false, server_error_code::invalid_message_type,
				minimal_serializer::generate_string(static_cast<int>(header.message_type)));
		}

		if (enable_message_specification && header.message_type != specified_message_type) {
			throw server_error(false, server_error_code::message_type_mismatch,
				minimal_serializer::generate_string("expected: ", specified_message_type, ", actual: ", header.message_type));
		}

		const auto message_handler = make_message_handler(header.message_type);
		const auto header_size = minimal_serializer::get_serialized_size<request_message_header>();
		const auto message_size = message_handler->get_message_size();
		log_with_endpoint(log_level::info, param->socket.remote_endpoint(), "Message header received. (type: ",
			header.message_type, ", size: ", header_size, ")");

		// Receive and process a body of message
		(*message_handler)(param);

		log_with_endpoint(log_level::info, param->socket.remote_endpoint(), "Message processed. (type: ",
			header.message_type, ", size: ", message_size, ")");
	}
}
