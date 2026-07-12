#pragma once

#include <boost/functional/hash.hpp>

#include "client_constants.hpp"

namespace pgl {
	/**
	 * A player full name. 26 bytes.
	 *
	 * There are four patterns for player name.
	 * - Full Name (Bill#123): Name and tag are not empty.
	 * - Name ("Bill"): Bill: Name is not empty and tag is empty.
	 * - Tag ("#123"): Name is empty and tag is not empty.
	 * - Empty (""): Name and tag are empty.
	 */
	struct player_full_name final {
		constexpr static player_tag_t not_assigned_tag = 0;
		
		/**
		 * A name of player. Empty name means name is not assigned.
		 */
		player_name_t name;

		/**
		 * A tag of player to identify same name player. 0 means tag is not assigned.
		 */
		player_tag_t tag;

		bool operator==(const player_full_name& other) const;
		bool operator!=(const player_full_name& other) const;

		/**
		 * Generate player full name string.
		 *
		 * @return A string of player full name.
		 */
		[[nodiscard]] std::string generate_full_name() const;

		/**
		 * Check if a tag is assigned.
		 *
		 * @return Whether a tag is assigned.
		 */
		[[nodiscard]] bool is_tag_assigned() const;

		/**
		 * Check if a name is assigned.
		 *
		 * @return Whether a name is assigned.
		 */
		[[nodiscard]] bool is_name_assigned() const;

		using serialize_targets = minimal_serializer::serialize_target_container<
			&player_full_name::name,
			&player_full_name::tag
		>;
	};

	inline size_t hash_value(const player_full_name& datetime) {
		size_t seed = 0;
		boost::hash_combine(seed, hash_value(datetime.name));
		boost::hash_combine(seed, boost::hash_value(datetime.tag));
		return seed;
	}
}

namespace boost {
	inline size_t hash_value(const pgl::player_full_name& datetime) {
		size_t seed = 0;
		hash_combine(seed, hash_value(datetime.name));
		hash_combine(seed, hash_value(datetime.tag));
		return seed;
	}
}

template <>
struct std::hash<pgl::player_full_name> {
	size_t operator()(const pgl::player_full_name& value) const noexcept { return boost::hash_value(value); }
};
