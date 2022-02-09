#pragma once

#include <boost/functional/hash.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/operators.hpp>

#include "minimal_serializer/serializer.hpp"

#include "network_layer.hpp"
#include "transport_layer.hpp"

namespace pgl {
	// 18 bytes
	struct endpoint final : private boost::equality_comparable<endpoint> {
		ip_address_type ip_address;
		port_number_type port_number;

		bool operator==(const endpoint& other) const;
		[[nodiscard]] ip_version ip_version()const;
		[[nodiscard]] boost::asio::ip::tcp::endpoint to_boost_endpoint()const;

		static endpoint make_from_boost_endpoint(
			const boost::asio::basic_socket<boost::asio::ip::tcp>::endpoint_type& boost_endpoint);

		using serialize_targets = minimal_serializer::serialize_target_container<
			&endpoint::ip_address,
			&endpoint::port_number
		>;
	};
}

namespace boost {
	inline size_t hash_value(const pgl::endpoint& endpoint) {
		size_t seed = 0;
		hash_combine(seed, hash_value(endpoint.ip_address));
		hash_combine(seed, hash_value(endpoint.port_number));
		return seed;
	}
}

template <>
struct std::hash<pgl::endpoint> {
	size_t operator()(const pgl::endpoint& endpoint) const noexcept {
		return boost::hash_value(endpoint);
	}
};
