#include "asio_stream_compatibility.hpp"

namespace pgl {
	std::ostream& operator<<(std::ostream& os,
	                         const boost::asio::basic_socket<boost::asio::ip::tcp>::endpoint_type& endpoint) {
		os << "IP address=" << endpoint.address().to_string() << ", port=" << endpoint.port();
		return os;
	}

	std::ostream& operator<<(std::ostream& os, const boost::system::error_code& error_code) {
		os << error_code.message();
		return os;
	}

	std::ostream& operator<<(std::ostream& os, const boost::system::system_error& error) {
		os << error.code();
		return os;
	}
}
