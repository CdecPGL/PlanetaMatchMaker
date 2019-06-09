#pragma once

#include <cstdint>

#include <boost/asio.hpp>
#include <boost/asio/spawn.hpp>

namespace pgl {
	template <typename...Data>
	using packed_data_buffer_t = std::array<uint8_t, (sizeof(Data) + ...)>;

	inline void pack_data2(uint8_t*, size_t) {}

	template <typename First, typename ... Rest>
	void pack_data2(uint8_t* buffer, const size_t pos, First&& first, Rest&&... rest) {
		std::memcpy(buffer + pos, &first, sizeof(first));
		pack_data2(buffer, pos + sizeof(first), rest...);
	}

	template <typename... Data>
	void pack_data(packed_data_buffer_t<Data...>& buffer, Data&&... data) {
		pack_data2(buffer.data(), 0, data...);
	}

	template <typename... Data>
	void packed_async_write(boost::asio::ip::tcp::socket& socket, boost::asio::yield_context& yield, Data& ... data) {
		packed_data_buffer_t<Data...> buffer;
		pack_data(buffer, data...);
		static_assert(sizeof(buffer) == (sizeof(data) + ...));
		async_write(socket, boost::asio::buffer(buffer.data(), sizeof(buffer)), yield);
	}
}
