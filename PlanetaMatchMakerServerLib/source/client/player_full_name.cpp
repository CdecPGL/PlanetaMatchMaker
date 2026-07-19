#include "player_full_name.hpp"

using namespace minimal_serializer;

namespace pgl {
	std::string player_full_name::generate_full_name() const { return generate_string(name, "#", tag); }
	bool player_full_name::is_tag_assigned() const { return tag != 0; }
	bool player_full_name::is_name_assigned() const { return name.length() != 0; }

	bool player_full_name::operator==(const player_full_name& other) const {
		return name == other.name && tag == other.tag;
	}

	bool player_full_name::operator!=(const player_full_name& other) const { return !(*this == other); }
}
