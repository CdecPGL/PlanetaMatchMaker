#pragma once

#include <cstdint>

#include "utilities/string_utility.hpp"
#include "serialize/serializer.hpp"

namespace pgl {
	template <typename T>
	using remove_cvref_t = std::remove_cv_t<std::remove_reference_t<T>>;

	template <typename... Data>
	size_t get_packed_size() {
		return (get_serialized_size<remove_cvref_t<Data>>() + ...);
	}

	inline void pack_data_impl(std::vector<uint8_t>&, size_t) {}

	template <typename First, typename ... Rest>
	void pack_data_impl(std::vector<uint8_t>& buffer, const size_t pos, const First& first, const Rest& ... rest) {
		auto serialized_data = serialize(first);
		auto size = serialized_data.size();
		if (pos + size > buffer.size()) {
			auto message = generate_string("Data range(pos=", pos, ", size=", size, ") exceeds the size of buffer(",
				buffer.size(), ").");
			throw serialization_error(message);
		}
		std::memcpy(buffer.data() + pos, serialized_data.data(), size);
		pack_data_impl(buffer, pos + size, rest...);
	}

	// Serialize multi data
	template <typename First, typename... Rest>
	std::vector<uint8_t> pack_data(const First& first, const Rest& ... rest) {
		// When there are only one data, serialize directory to avoid redundant buffer copy
		if constexpr (sizeof...(Rest) == 0) {
			return serialize(first);
		} else {
			auto total_size = get_packed_size<First, Rest...>();
			std::vector<uint8_t> buffer(total_size);
			pack_data_impl(buffer, 0, first, rest...);
			return buffer;
		}
	}

	inline void unpack_data_impl(const std::vector<uint8_t>&, size_t) {}

	template <typename First, typename ... Rest>
	void unpack_data_impl(const std::vector<uint8_t>& buffer, const size_t pos, First& first, Rest& ... rest) {
		auto size = get_serialized_size<First>();
		if (pos + size > buffer.size()) {
			auto message = generate_string("Data range(pos=", pos, ", size=", size, ") exceeds the size of buffer(",
				buffer.size(), ").");
			throw serialization_error(message);
		}
		std::vector<uint8_t> data(size);
		std::memcpy(data.data(), buffer.data() + pos, size);
		deserialize(first, data);
		unpack_data_impl(buffer, pos + size, rest...);
	}

	// Deserialize multi data
	template <typename First, typename ... Rest>
	std::vector<uint8_t> unpack_data(const std::vector<uint8_t>& buffer, First& first, Rest& ... rest) {
		// When there are only one data, deserialize directory to avoid redundant buffer copy
		if constexpr (sizeof...(Rest) == 0) {
			deserialize(first, buffer);
		} else {
			unpack_data_impl(buffer, 0, first, rest...);
		}
		return buffer;
	}
}
