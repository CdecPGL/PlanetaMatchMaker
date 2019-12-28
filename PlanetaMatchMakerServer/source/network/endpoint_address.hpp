#pragma once

#include <boost/functional/hash.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/operators.hpp>

#include "minimal_serializer/serializer.hpp"

#include "network_layer.hpp"
#include "transport_layer.hpp"

namespace pgl {
	// 18 bytes
	struct endpoint_address final : private boost::equality_comparable<endpoint_address> {
		ip_address_type ip_address;
		port_number_type port_number;

		bool operator==(const endpoint_address& other) const;

		ip_version ip_version()const;

		static endpoint_address make_from_boost_endpoint(
			const boost::asio::basic_socket<boost::asio::ip::tcp>::endpoint_type& endpoint);

		void on_serialize(minimal_serializer::serializer& serializer) {
			serializer += ip_address;
			serializer += port_number;
		}
	};
}

namespace boost {
	inline size_t hash_value(const pgl::endpoint_address& client_address) {
		size_t seed = 0;
		hash_combine(seed, boost::hash_value(client_address.ip_address));
		hash_combine(seed, hash_value(client_address.port_number));
		return seed;
	}
}

namespace std {
	template <>
	struct hash<pgl::endpoint_address> {
		size_t operator()(const pgl::endpoint_address& client_address) const noexcept {
			return boost::hash_value(client_address);
		}
	};
}
