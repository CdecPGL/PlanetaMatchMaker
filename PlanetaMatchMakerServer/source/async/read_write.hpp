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
	void packed_async_write(boost::asio::ip::tcp::socket& socket, boost::asio::yield_context& yield, Data&& ... data) {
		auto buffer = pack_data(std::forward<Data>(data)...);
		async_write(socket, boost::asio::buffer(buffer), yield);
	}
}
