#pragma once

#include <boost/asio.hpp>
#include <boost/asio/spawn.hpp>

#include "utilities/string_utility.hpp"
#include "serialize/pack.hpp"

namespace pgl {
	/*
	 * Send multi serializable data in one communication
	 * todo: Reduce redundant copy. Currently, there are two copy steps (copy in serialization and copy in pack_data)
	 */
	template <typename... Data>
	void packed_async_write(boost::asio::ip::tcp::socket& socket, boost::asio::yield_context& yield,
		const Data& ... data) {
		auto buffer = pack_data(data...);
		async_write(socket, boost::asio::buffer(buffer), yield);
	}

	/*
	 * Receive multi serializable data in one communication
	 * todo: Reduce redundant copy. Currently, there are two copy steps (copy in serialization and copy in pack_data)
	 */
	template <typename... Data>
	void unpacked_async_read(boost::asio::ip::tcp::socket& socket, boost::asio::yield_context& yield, Data& ... data) {
		static_assert(!(std::is_const_v<Data> || ...),"All Data must not be const.");
		auto total_size = get_packed_size<Data...>();
		std::vector<uint8_t> buffer(total_size);
		async_read(socket, boost::asio::buffer(buffer), boost::asio::transfer_exactly(total_size), yield);
		unpack_data(buffer, data...);
	}
}
