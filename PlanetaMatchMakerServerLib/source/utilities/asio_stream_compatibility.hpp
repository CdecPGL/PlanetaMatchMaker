#pragma once

#include <boost/asio.hpp>

namespace boost {
	namespace asio {
		std::ostream& operator <<(std::ostream& os, const basic_socket<ip::tcp>::endpoint_type& endpoint);
	}
	namespace system {
		std::ostream& operator <<(std::ostream& os, const error_code& error_code);
		std::ostream& operator <<(std::ostream& os, const system_error& error);
	}
}
