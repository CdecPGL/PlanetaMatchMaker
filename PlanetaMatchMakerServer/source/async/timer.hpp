#pragma once

#include <atomic>
#include <chrono>
#include <functional>
#include <memory>

#include <boost/asio.hpp>

namespace pgl {
	// The executor must serialize this timer handler with socket operations, normally by constructing the socket
	// with a strand. This keeps cancel() ordered with the yielding async operation.
	template <class SocketLike, typename... TimeParams>
	void execute_socket_timed_async_operation(SocketLike& socket,
		const std::chrono::duration<TimeParams...>& time, std::function<void()>&& proc) {
		boost::asio::steady_timer timer(socket.get_executor());
		timer.expires_after(time);
		auto is_operation_finished = std::make_shared<std::atomic_bool>(false);
		timer.async_wait([&socket, is_operation_finished](const boost::system::error_code& error_code) {
			if (!error_code && !is_operation_finished->exchange(true, std::memory_order_acq_rel)) {
				boost::system::error_code ignored_error;
				socket.cancel(ignored_error);
			}
		});

		const auto finish_timer = [&timer, is_operation_finished] {
			if (!is_operation_finished->exchange(true, std::memory_order_acq_rel)) {
				try {
					static_cast<void>(timer.cancel());
				}
				catch (const boost::system::system_error&) {
					// Match the old cancel(error_code) behavior by ignoring cancellation errors.
				}
			}
		};

		try { proc(); }
		catch (const boost::system::system_error&) {
			finish_timer();
			throw;
		}

		finish_timer();
	}
}
