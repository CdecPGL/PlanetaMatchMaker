#include "transport_layer.hpp"

namespace pgl {
	boost::asio::ip::tcp get_tcp(const ip_version ip_version) {
		switch (ip_version) {
		case ip_version::v4: return boost::asio::ip::tcp::v4();
		case ip_version::v6: return boost::asio::ip::tcp::v6();
		default: throw std::runtime_error("Invalid IP version.");
		}
	}
}