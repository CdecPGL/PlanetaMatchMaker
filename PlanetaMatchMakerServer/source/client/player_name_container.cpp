#include <limits>

#include "minimal_serializer/string_utility.hpp"

#include "player_name_container.hpp"

using namespace std;
using namespace minimal_serializer;

namespace pgl {
	player_full_name player_name_container::assign_player_name(const player_name_t& player_name) {
		lock_guard lock(mutex_);
		auto it = name_map_.find(player_name);
		if (it == name_map_.end()) {
			name_map_.emplace(player_name, name_data{1, {1}});
			return player_full_name{player_name, 0};
		}

		if (it->second.used_tags.size() >= numeric_limits<player_tag_t>::max()) {
			throw player_name_error(generate_string("The number of player tag for \"", player_name,
				"\" reaches limit (", numeric_limits<player_tag_t>::max(), ")."));
		}

		while (it->second.used_tags.find(it->second.next_tag) == it->second.used_tags.end()) { ++it->second.next_tag; }
		const auto tag = it->second.next_tag;
		it->second.used_tags.insert(tag);
		return player_full_name{player_name, tag};
	}

	void player_name_container::remove_player_name(const player_full_name& player_full_name) {
		lock_guard lock(mutex_);
		auto it = name_map_.find(player_full_name.name);
		if (it == name_map_.end()) {
			throw player_name_error(generate_string("The player full name (name: ", player_full_name.name, ", tag: ",
				player_full_name.tag, ") does not exist."));
		}

		const auto tag_it = it->second.used_tags.find(player_full_name.tag);
		if (tag_it == it->second.used_tags.end()) {
			throw player_name_error(generate_string("The player full name (name: ", player_full_name.name, ", tag: ",
				player_full_name.tag, ") does not exist."));
		}

		it->second.used_tags.erase(tag_it);
	}

	player_name_error::player_name_error(const std::string& message): runtime_error(message) {}
}
