#pragma once

#include <boost/functional/hash.hpp>

#include "client_constants.hpp"

namespace pgl {
	// 26 bytes
	struct player_full_name final {
		const static player_tag_t not_assigned_tag = 0;
		
		// A name of player. Empty name means name is not assigned.
		player_name_t name;

		// A tag of player to identify same name player. 0 means tag is not assigned.
		player_tag_t tag;

		bool operator==(const player_full_name& other) const;
		bool operator!=(const player_full_name& other) const;

		[[nodiscard]] std::string generate_full_name() const;
		[[nodiscard]] bool is_tag_assigned() const;
		[[nodiscard]] bool is_name_assigned() const;

		void on_serialize(minimal_serializer::serializer& serializer) {
			serializer += name;
			serializer += tag;
		}
	};
}

namespace boost {
	inline size_t hash_value(const pgl::player_full_name& datetime) {
		size_t seed = 0;
		hash_combine(seed, hash_value(datetime.name));
		hash_combine(seed, hash_value(datetime.tag));
		return seed;
	}
}

namespace std {
	template <>
	struct hash<pgl::player_full_name> {
		size_t operator()(const pgl::player_full_name& value) const noexcept { return boost::hash_value(value); }
	};
}
