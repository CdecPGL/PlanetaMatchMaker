#pragma once

#include <boost/asio.hpp>

namespace pgl {
	std::ostream& operator <<(std::ostream& os,
		const boost::asio::basic_socket<boost::asio::ip::tcp>::endpoint_type& endpoint);
	std::ostream& operator <<(std::ostream& os, const boost::system::error_code& error_code);
	std::ostream& operator <<(std::ostream& os, const boost::system::system_error& error);
}
