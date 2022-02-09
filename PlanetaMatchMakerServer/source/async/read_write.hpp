#pragma once

#include <boost/asio.hpp>
#include <boost/asio/spawn.hpp>

#include "utilities/pack.hpp"
#include "utilities/concepts.hpp"

namespace pgl {
	/*
	 * Send multi serializable data in one communication
	 * todo: Reduce redundant copy. Currently, there are two copy steps (copy in serialization and copy in pack_data)
	 */
	template <serializable... Data>
	void packed_async_write(boost::asio::ip::tcp::socket& socket, boost::asio::yield_context yield,
		const Data& ... data) {
		auto buffer = pack_data(data...);
		async_write(socket, boost::asio::buffer(buffer), yield);
	}

	/*
	 * Receive multi serializable data in one communication
	 * todo: Reduce redundant copy. Currently, there are two copy steps (copy in serialization and copy in pack_data)
	 */
	template <typename... Data> requires(serializable_all<Data...> && not_constant_all<Data...>)
	void unpacked_async_read(boost::asio::ip::tcp::socket& socket, boost::asio::yield_context yield, Data& ... data) {
		constexpr auto total_size = get_packed_size<Data...>();
		std::vector<uint8_t> buffer(total_size);
		async_read(socket, boost::asio::buffer(buffer), boost::asio::transfer_exactly(total_size), yield);
		unpack_data(buffer, data...);
	}
}
