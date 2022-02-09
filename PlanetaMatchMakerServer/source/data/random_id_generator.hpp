#pragma once

#include <random>
#include <limits>

namespace pgl {
	// Generate random ID thread safely with referring the type of ID.
	template <std::integral Id>
	Id generate_random_id() {
		static thread_local std::mt19937_64 mt((std::random_device())());
		std::uniform_int_distribution<Id> uniform_distribution(std::numeric_limits<Id>::min(),
			std::numeric_limits<Id>::max());
		return uniform_distribution(mt);
	}
}
