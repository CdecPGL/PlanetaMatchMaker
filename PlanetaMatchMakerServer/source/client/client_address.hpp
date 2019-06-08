#pragma once

#include <boost/functional/hash.hpp>

#include "network/network_layer.hpp"
#include "network/transport_layer.hpp"

namespace pgl {
	// 18 bytes
	struct client_address {
		ip_address_type ip_address{};
		port_number_type port_number{};
	};
}

namespace boost {
	inline size_t hash_value(const pgl::client_address& client_address) {
		size_t seed = 0;
		boost::hash_combine(seed, boost::hash_value(client_address.ip_address));
		boost::hash_combine(seed, boost::hash_value(client_address.port_number));
		return seed;
	}
}

namespace std {
	template <>
	struct hash<pgl::client_address> {
		size_t operator()(const pgl::client_address& client_address) const noexcept {
			return boost::hash_value(client_address);
		}
	};
}
