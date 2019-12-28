#include "endpoint_address.hpp"

namespace pgl {
	constexpr std::array<uint8_t, 12> ipv4_prefix = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xff, 0xff };

	bool endpoint_address::operator==(const endpoint_address& other) const {
		return ip_address == other.ip_address && port_number == other.port_number;
	}

	ip_version endpoint_address::ip_version() const {
		if (std::equal(ipv4_prefix.begin(), ipv4_prefix.end(), ip_address.begin())) {
			return ip_version::v4;
		}

		return ip_version::v6;
	}

	endpoint_address endpoint_address::make_from_boost_endpoint(
		const boost::asio::basic_socket<boost::asio::ip::tcp>::endpoint_type& endpoint) {
		endpoint_address client_address{};
		if (endpoint.address().is_v4()) {
			// IPv4-Mapped IPv6 Address (example, ::ffff:192.0.0.1)
			auto bytes = endpoint.address().to_v4().to_bytes();
			std::memcpy(client_address.ip_address.data(), ipv4_prefix.data(), ipv4_prefix.size());
			std::memcpy(client_address.ip_address.data() + 12, bytes.data(), bytes.size());
		} else {
			auto bytes = endpoint.address().to_v6().to_bytes();
			std::memcpy(client_address.ip_address.data(), bytes.data(), bytes.size());
		}

		client_address.port_number = endpoint.port();
		return client_address;
	}
}
