#include <boost/test/unit_test.hpp>

#include "server/server_tls_reload_signal_handler.hpp"

#include <atomic>
#include <chrono>
#include <thread>

#ifndef _WIN32
#include <csignal>
#include <unistd.h>
#endif

using namespace boost;
using namespace pgl;

BOOST_AUTO_TEST_SUITE(server_tls_reload_signal_handler_test)

BOOST_AUTO_TEST_CASE(reload_callback_is_called_when_sighup_is_received) {
#ifdef _WIN32
	BOOST_TEST_MESSAGE("SIGHUP reload is not supported on Windows.");
#else
	asio::io_context io_context;
	std::atomic_bool reloaded = false;

	server_tls_reload_signal_handler signal_handler(io_context, [&] {
		reloaded = true;
		io_context.stop();
	});
	signal_handler.start();

	std::thread io_thread([&] { io_context.run(); });
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	const auto signal_sent = kill(getpid(), SIGHUP) == 0;

	const auto timeout = std::chrono::steady_clock::now() + std::chrono::seconds(5);
	while (!reloaded && std::chrono::steady_clock::now() < timeout) {
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	io_context.stop();
	if (io_thread.joinable()) { io_thread.join(); }

	BOOST_CHECK(signal_sent);
	BOOST_CHECK(reloaded);
#endif
}

BOOST_AUTO_TEST_SUITE_END()
