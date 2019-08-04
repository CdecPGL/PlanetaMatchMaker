#include "data/random_id_generator.hpp"

#include "session_key.hpp"

namespace pgl {
	session_key_type generate_random_session_key() {
		return generate_random_id<session_key_type>();
	}
}
