#pragma once

#include <boost/asio.hpp>
#include <boost/asio/spawn.hpp>

#include "utilities/pack.hpp"
#include "utilities/concepts.hpp"
#include "network/client_connection.hpp"

namespace pgl {
	/*
	 * Send multi serializable data in one communication
	 */
	template <serializable... Data>
	void packed_async_write(client_connection& connection, boost::asio::yield_context yield,
		const Data& ... data) {
		auto buffer = pack_data(data...);
		connection.async_write(boost::asio::buffer(buffer), yield);
	}

	/*
	 * Receive multi serializable data in one communication
	 */
	template <typename... Data> requires(serializable_all<Data...> && not_constant_all<Data...>)
	void unpacked_async_read(client_connection& connection, boost::asio::yield_context yield, Data& ... data) {
		constexpr auto total_size = get_packed_size<Data...>();
		std::vector<uint8_t> buffer(total_size);
		connection.async_read(boost::asio::buffer(buffer), boost::asio::transfer_exactly(total_size), yield);
		unpack_data(buffer, data...);
	}
}
