#pragma once

#include <unordered_map>
#include <unordered_set>
#include <shared_mutex>

#include "player_full_name.hpp"

namespace pgl {
	// A thread safe container to manage player name
	class player_name_container final {
	public:
		// Assign new player and get player name.
		// Throws player_name_error if player tag reaches limit.
		player_full_name assign_player_name(const player_name_t& player_name);

		// Remove player name.
		// Throws player_name_error if player name or tag does not exist.
		void remove_player_name(const player_full_name& player_full_name);

		[[nodiscard]] bool is_player_exist(const player_full_name& player_full_name) const;
	private:
		struct name_data {
			player_tag_t next_tag;
			std::unordered_set<player_tag_t> used_tags;
		};

		using name_map_t = std::unordered_map<player_name_t, name_data>;
		name_map_t name_map_;
		mutable std::shared_mutex mutex_;
	};

	class player_name_error final : public std::runtime_error {
	public:
		explicit player_name_error(const std::string& message);
	};
}
