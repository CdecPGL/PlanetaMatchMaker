#include "endpoint.hpp"

using namespace boost;

namespace pgl {
	constexpr std::array<uint8_t, 12> ipv4_prefix = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xff, 0xff};

	bool endpoint::operator==(const endpoint& other) const {
		return ip_address == other.ip_address && port_number == other.port_number;
	}

	ip_version endpoint::ip_version() const {
		if (std::equal(ipv4_prefix.begin(), ipv4_prefix.end(), ip_address.begin())) { return ip_version::v4; }

		return ip_version::v6;
	}

	asio::ip::tcp::endpoint endpoint::to_boost_endpoint() const {
		asio::ip::address boost_address;
		switch (ip_version()) {
			case ip_version::v4:
				asio::ip::address_v4::bytes_type ip_address_v4;
				std::memcpy(ip_address_v4.data(), ip_address.data() + 12, ip_address_v4.size());
				boost_address = asio::ip::address_v4(ip_address_v4);
				break;
			case ip_version::v6:
				boost_address = asio::ip::address_v6(ip_address);
				break;
			default:
				assert(false);
				break;
		}

		return { boost_address, port_number };
	}

	endpoint endpoint::make_from_boost_endpoint(
		const asio::basic_socket<asio::ip::tcp>::endpoint_type& boost_endpoint) {
		endpoint endpoint{};
		if (boost_endpoint.address().is_v4()) {
			// IPv4-Mapped IPv6 Address (example, ::ffff:192.0.0.1)
			const auto bytes = boost_endpoint.address().to_v4().to_bytes();
			std::memcpy(endpoint.ip_address.data(), ipv4_prefix.data(), ipv4_prefix.size());
			std::memcpy(endpoint.ip_address.data() + 12, bytes.data(), bytes.size());
		}
		else {
			const auto bytes = boost_endpoint.address().to_v6().to_bytes();
			std::memcpy(endpoint.ip_address.data(), bytes.data(), bytes.size());
		}

		endpoint.port_number = boost_endpoint.port();
		return endpoint;
	}
}
