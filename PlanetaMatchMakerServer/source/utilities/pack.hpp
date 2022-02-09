#pragma once

#include <cstdint>

#include "minimal_serializer/string_utility.hpp"
#include "minimal_serializer/serializer.hpp"

#include "concepts.hpp"

namespace pgl {
	/**
	 * Get a size of serialized multiple data.
	 *
	 * @tparam Data A type of data.
	 * @return A size of serialized multiple data.
	 */
	template <serializable... Data>
	constexpr size_t get_packed_size() { return (minimal_serializer::serialized_size_v<Data> + ...); }

	inline void pack_data_impl(std::vector<uint8_t>&, size_t) {}

	template <serializable First, serializable ... Rests>
	void pack_data_impl(std::vector<uint8_t>& buffer, const size_t pos, const First& first, const Rests& ... rests) {
		constexpr auto size = minimal_serializer::serialized_size_v<First>;
		if (pos + size > buffer.size()) {
			auto message = minimal_serializer::generate_string("Data range(pos=", pos, ", size=", size,
				") exceeds the size of buffer(", buffer.size(), ") in data packing.");
			throw minimal_serializer::serialization_error(message);
		}
		minimal_serializer::serialize(first, buffer, pos);
		pack_data_impl(buffer, pos + size, rests...);
	}

	/**
	 * Serialize multiple data.
	 *
	 * @param first First data to serialize.
	 * @param rests Rest data to serialize.
	 * @tparam First A type of first data.
	 * @tparam Rests Types of rest data.
	 * @return A byte array of serialized data.
	 * @exception minimal_serializer::serialization_error Failed to serialize.
	 */
	template <serializable First, serializable... Rests>
	std::vector<uint8_t> pack_data(const First& first, const Rests& ... rests) {
		constexpr auto total_size = get_packed_size<First, Rests...>();
		std::vector<uint8_t> buffer(total_size);
		pack_data_impl(buffer, 0, first, rests...);
		return buffer;
	}

	inline void unpack_data_impl(const std::vector<uint8_t>&, size_t) {}

	template <serializable First, serializable ... Rests>
	void unpack_data_impl(const std::vector<uint8_t>& buffer, const size_t pos, First& first, Rests& ... rests) {
		constexpr auto size = minimal_serializer::serialized_size_v<First>;
		if (pos + size > buffer.size()) {
			auto message = minimal_serializer::generate_string("Data range(pos=", pos, ", size=", size,
				") exceeds the size of buffer(", buffer.size(), ") in data unpacking.");
			throw minimal_serializer::serialization_error(message);
		}

		minimal_serializer::deserialize(first, buffer, pos);
		unpack_data_impl(buffer, pos + size, rests...);
	}

	/**
	 * Deserialize multiple data.
	 *
	 * @param buffer A byte array of serialized data.
	 * @param first First data to deserialize.
	 * @param rests Rest data to deserialize.
	 * @tparam First A type of first data.
	 * @tparam Rests Types of rest data.
	 * @exception minimal_serializer::serialization_error Failed to deserialize.
	 */
	template <typename First, typename ... Rests> requires(serializable_all<First, Rests...>&& not_constant_all<First, Rests...>)
	void unpack_data(const std::vector<uint8_t>& buffer, First& first, Rests& ... rests) {
		unpack_data_impl(buffer, 0, first, rests...);
	}
}
