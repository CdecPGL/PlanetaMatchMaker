#pragma once

#include <random>
#include <limits>

namespace pgl {
	template <typename T>
	concept eight_bit_integral = std::same_as<T, uint8_t> || std::same_as<T, int8_t>;

	// Generate random ID thread safely with referring the type of ID.
	template <std::integral Id> requires(!eight_bit_integral<Id>)
	Id generate_random_id() {
		static thread_local std::mt19937_64 mt((std::random_device())());
		std::uniform_int_distribution<Id> uniform_distribution(std::numeric_limits<Id>::min(),
			std::numeric_limits<Id>::max());
		return uniform_distribution(mt);
	}

	template <std::integral Id> requires(eight_bit_integral<Id>)
	Id generate_random_id() {
		static thread_local std::mt19937_64 mt((std::random_device())());
		// std::uniform_int_distribution is not compatible for char, signed char, unsigned char, char8_t, int8_t, and uint8_t, so use larger type and cast target type
		std::uniform_int_distribution<int32_t> uniform_distribution(std::numeric_limits<Id>::min(),
			std::numeric_limits<Id>::max());
		return uniform_distribution(mt);
	}
}