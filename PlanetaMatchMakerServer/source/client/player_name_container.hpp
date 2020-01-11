#pragma once

#include <unordered_map>
#include <unordered_set>
#include <shared_mutex>

#include "player_full_name.hpp"

namespace pgl {
	// A thread safe container to manage player name
	class player_name_container final {
	public:
		// assign new player and get player name.
		// throws player_name_error if player tag reaches limit.
		player_full_name assign_player_name(const player_name_t& player_name);

		// remove player name.
		// throws player_name_error if player name or tag does not exist.
		void remove_player_name(const player_full_name& player_full_name);
	private:
		struct name_data {
			player_tag_t next_tag;
			std::unordered_set<player_tag_t> used_tags;
		};

		using name_map_t = std::unordered_map<player_name_t, name_data>;
		name_map_t name_map_;
		std::shared_mutex mutex_;
	};

	class player_name_error final : public std::runtime_error {
	public:
		explicit player_name_error(const std::string& message);
	};
}
