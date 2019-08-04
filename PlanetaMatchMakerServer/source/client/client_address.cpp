#include "client_address.hpp"

namespace pgl {
	bool client_address::operator==(const client_address& other) const {
		return ip_address == other.ip_address && port_number == other.port_number;
	}

	client_address client_address::make_from_endpoint(
		const boost::asio::basic_socket<boost::asio::ip::tcp>::endpoint_type& endpoint) {
		client_address client_address{};
		if (endpoint.address().is_v4()) {
			auto bytes = endpoint.address().to_v4().to_bytes();
			std::memcpy(client_address.ip_address.data(), bytes.data(), bytes.size());
		} else {
			auto bytes = endpoint.address().to_v6().to_bytes();
			std::memcpy(client_address.ip_address.data(), bytes.data(), bytes.size());
		}

		client_address.port_number = endpoint.port();
		return client_address;
	}
}
