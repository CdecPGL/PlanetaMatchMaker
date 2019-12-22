#pragma once

#include <chrono>
#include <functional>

#include <boost/asio.hpp>

namespace pgl {
	template <typename... TimeParams>
	void execute_timed_async_operation(boost::asio::ip::tcp::socket& socket,
		const std::chrono::duration<TimeParams...>& time, std::function<void()>&& proc) {
		boost::asio::steady_timer timer(socket.get_executor());
		timer.expires_after(time);
		auto is_timer_canceled = std::make_shared<bool>(false);
		timer.async_wait([&socket, is_timer_canceled](const boost::system::error_code& error_code) {
			if (!error_code && !*is_timer_canceled) {
				socket.cancel();
			}
		});

		try {
			proc();
		} catch (const boost::system::system_error& e) {
			if (e.code() != boost::asio::error::operation_aborted) {
				*is_timer_canceled = true;
				timer.cancel();
			}
			throw;
		}

		*is_timer_canceled = true;
		timer.cancel();
	}
}
