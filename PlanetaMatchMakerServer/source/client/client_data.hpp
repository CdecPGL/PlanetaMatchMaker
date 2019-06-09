#pragma once

#include "datetime/datetime.hpp"

#include "client_address.hpp"

namespace pgl {
	struct client_data final {
		client_address address{};
		datetime last_communicate_datetime{};
	};
}

namespace boost {
	inline size_t hash_value(const pgl::client_data& client_data) {
		size_t seed = 0;
		hash_combine(seed, hash_value(client_data.address));
		hash_combine(seed, hash_value(client_data.last_communicate_datetime));
		return seed;
	}
}

namespace std {
	template <>
	struct hash<pgl::client_data> {
		size_t operator()(const pgl::client_data& client_data) const noexcept {
			return boost::hash_value(client_data);
		}
	};
}
