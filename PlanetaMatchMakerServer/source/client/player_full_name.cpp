#include "player_full_name.hpp"

using namespace minimal_serializer;

namespace pgl {
	std::string player_full_name::generate_full_name() const { return generate_string(name, "#", tag); }
}
