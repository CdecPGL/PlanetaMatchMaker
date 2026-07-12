#pragma once

#include <chrono>

#include <boost/asio/any_io_executor.hpp>
#include <boost/asio/spawn.hpp>

namespace pgl {
	struct authentication_execution_context final {
		boost::asio::any_io_executor executor;
		boost::asio::yield_context yield;
		std::chrono::steady_clock::time_point deadline;
		bool allow_plain_http = false;
	};
}
