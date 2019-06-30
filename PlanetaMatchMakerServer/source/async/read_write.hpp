#pragma once

#include <cstdint>

#include <boost/asio.hpp>
#include <boost/asio/spawn.hpp>

#include "utilities/string_utility.hpp"
#include "serialize/serializer.hpp"

namespace pgl {
	template <typename T>
	using remove_cvref_t = std::remove_cv_t<std::remove_reference_t<T>>;

	inline void pack_data_impl(std::vector<uint8_t>&, size_t) {}

	template <typename First, typename ... Rest>
	void pack_data_impl(std::vector<uint8_t>& buffer, const size_t pos, First&& first, Rest&&... rest) {
		auto serialized_data = serialize(first);
		auto size = serialized_data.size();
		if (pos + size > buffer.size()) {
			auto message = generate_string("Data range(pos=", pos, ", size=", size, ") exceeds the size of buffer(",
				buffer.size(), ").");
			throw std::runtime_error(message);
		}
		std::memcpy(buffer.data() + pos, serialized_data.data(), size);
		pack_data_impl(buffer, pos + size, std::forward<Rest>(rest)...);
	}

	// Serialize multi data
	template <typename... Data>
	std::vector<uint8_t> pack_data(Data&&... data) {
		auto total_size = (get_serialized_size<remove_cvref_t<Data>>() + ...);
		std::vector<uint8_t> buffer(total_size);
		pack_data_impl(buffer, 0, std::forward<Data>(data)...);
		return buffer;
	}

	inline void unpack_data_impl(const std::vector<uint8_t>&, size_t) {}

	template <typename First, typename ... Rest>
	void unpack_data_impl(const std::vector<uint8_t>& buffer, const size_t pos, First&& first, Rest&& ... rest) {
		auto size = get_serialized_size<First>();
		if (pos + size > buffer.size()) {
			auto message = generate_string("Data range(pos=", pos, ", size=", size, ") exceeds the size of buffer(",
				buffer.size(), ").");
			throw std::runtime_error(message);
		}
		std::vector<uint8_t> data(size);
		std::memcpy(data.data(), buffer.data() + pos, size);
		deserialize(first, data);
		unpack_data_impl(buffer, pos + size, std::forward<Rest>(rest)...);
	}

	// Deserialize multi data
	template <typename... Data>
	std::vector<uint8_t> unpack_data(const std::vector<uint8_t>& buffer, Data& ... data) {
		unpack_data_impl(buffer, 0, std::forward<Data>(data)...);
		return buffer;
	}

	/*
	 * Send multi serializable data in one communication
	 * todo: Reduce redundant copy. Currently, there are two copy steps (copy in serialization and copy in pack_data)
	 */
	template <typename... Data>
	void packed_async_write(boost::asio::ip::tcp::socket& socket, boost::asio::yield_context& yield, Data&& ... data) {
		auto buffer = pack_data(std::forward<Data>(data)...);
		async_write(socket, boost::asio::buffer(buffer), yield);
	}
}
