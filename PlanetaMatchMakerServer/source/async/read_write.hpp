#pragma once

#include <boost/asio.hpp>
#include <boost/asio/spawn.hpp>

namespace pgl {
	template <typename... Data>
	void packed_async_write(boost::asio::ip::tcp::socket& socket, boost::asio::yield_context& yield, Data& ... data) {
		auto packed_data = std::make_tuple(data...);
		using packed_type = decltype(packed_data);
		static_assert((sizeof(data) + ...) == sizeof(packed_type));
		async_write(socket, boost::asio::buffer(&packed_data, sizeof(packed_type)), yield);
	}
}
