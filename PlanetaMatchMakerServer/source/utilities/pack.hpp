#pragma once

#include <cstdint>

#include "minimal_serializer/string_utility.hpp"
#include "minimal_serializer/serializer.hpp"

namespace pgl {
	template <typename... Data>
	size_t get_packed_size() {
		return (minimal_serializer::get_serialized_size<minimal_serializer::remove_cvref_t<Data>>() + ...);
	}

	inline void pack_data_impl(std::vector<uint8_t>&, size_t) {}

	template <typename First, typename ... Rests>
	void pack_data_impl(std::vector<uint8_t>& buffer, const size_t pos, const First& first, const Rests& ... rests) {
		auto serialized_data = minimal_serializer::serialize(first);
		auto size = serialized_data.size();
		if (pos + size > buffer.size()) {
			auto message = minimal_serializer::generate_string("Data range(pos=", pos, ", size=", size, ") exceeds the size of buffer(",
				buffer.size(), ") in data packing.");
			throw minimal_serializer::serialization_error(message);
		}
		std::memcpy(buffer.data() + pos, serialized_data.data(), size);
		pack_data_impl(buffer, pos + size, rests...);
	}

	// Serialize multi data
	template <typename First, typename... Rests>
	std::vector<uint8_t> pack_data(const First& first, const Rests& ... rests) {
		// When there are only one data, serialize directory to avoid redundant buffer copy
		if constexpr (sizeof...(Rests) == 0) {
			return minimal_serializer::serialize(first);
		} else {
			auto total_size = get_packed_size<First, Rests...>();
			std::vector<uint8_t> buffer(total_size);
			pack_data_impl(buffer, 0, first, rests...);
			return buffer;
		}
	}

	inline void unpack_data_impl(const std::vector<uint8_t>&, size_t) {}

	template <typename First, typename ... Rests>
	void unpack_data_impl(const std::vector<uint8_t>& buffer, const size_t pos, First& first, Rests& ... rests) {
		auto size = minimal_serializer::get_serialized_size<First>();
		if (pos + size > buffer.size()) {
			auto message = minimal_serializer::generate_string("Data range(pos=", pos, ", size=", size, ") exceeds the size of buffer(",
				buffer.size(), ") in data unpacking.");
			throw minimal_serializer::serialization_error(message);
		}
		std::vector<uint8_t> data(size);
		std::memcpy(data.data(), buffer.data() + pos, size);
		minimal_serializer::deserialize(first, data);
		unpack_data_impl(buffer, pos + size, rests...);
	}

	// Deserialize multi data
	template <typename First, typename ... Rests>
	void unpack_data(const std::vector<uint8_t>& buffer, First& first, Rests& ... rests) {
		static_assert(!(std::is_const_v<First> || (std::is_const_v<Rests> || ...)),
			"First and all Rests must not be const.");
		// When there are only one data, deserialize directory to avoid redundant buffer copy
		if constexpr (sizeof...(Rests) == 0) {
			minimal_serializer::deserialize(first, buffer);
		} else {
			unpack_data_impl(buffer, 0, first, rests...);
		}
	}
}
