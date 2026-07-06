#include "server_tls_reload_signal_handler.hpp"

#ifndef _WIN32
#include <csignal>
#endif

#include <utility>

namespace pgl {
	server_tls_reload_signal_handler::server_tls_reload_signal_handler(
		boost::asio::io_context& io_context, reload_callback reload):
		signals_(io_context), reload_(std::move(reload)) {
#ifndef _WIN32
		signals_.add(SIGHUP);
#endif
	}

	void server_tls_reload_signal_handler::start() {
#ifndef _WIN32
		wait();
#endif
	}

	void server_tls_reload_signal_handler::wait() {
#ifndef _WIN32
		signals_.async_wait([this](const boost::system::error_code& error, const int signal_number) {
			if (error) { return; }
			if (signal_number == SIGHUP) { reload_(); }
			wait();
		});
#endif
	}
}
