#include "asio_stream_compatibility.hpp"

namespace boost {
	namespace asio {
		std::ostream& operator<<(std::ostream& os, const basic_socket<ip::tcp>::endpoint_type& endpoint) {
			os << "endpoint(" << endpoint.address().to_string() << ":" << endpoint.port() << ")";
			return os;
		}
	}

	namespace system {
		std::ostream& operator<<(std::ostream& os, const error_code& error_code) {
			os << error_code.message();
			return os;
		}

		std::ostream& operator<<(std::ostream& os, const system_error& error) {
			os << error.code();
			return os;
		}
	}
}
