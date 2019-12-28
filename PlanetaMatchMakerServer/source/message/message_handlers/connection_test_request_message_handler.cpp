#include <boost/asio.hpp>

#include "server/server_data.hpp"
#include "server/server_constants.hpp"
#include "server/server_error.hpp"
#include "session/session_data.hpp"
#include "utilities/log.hpp"
#include "../message_handle_utilities.hpp"
#include "connection_test_request_message_handler.hpp"

using namespace boost;

namespace pgl {
	void connection_test_request_message_handler::handle_message(const connection_test_request_message& message,
		std::shared_ptr<message_handle_parameter> param) {

		connection_test_reply_message reply{};

		// Try to connect TCP server in the client
		asio::io_service io_service;
		asio::ip::tcp::socket socket(io_service);
		socket.async_connect(asio::ip::tcp::endpoint(asio::ip::address::from_string("127.0.0.1"), message.port_number), param->yield);


		reply_message_header header{
			message_type::connection_test_reply,
			message_error_code::ok
		};
	}
}
