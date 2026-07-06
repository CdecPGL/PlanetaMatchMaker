#pragma once

#include <functional>

#include <boost/asio.hpp>
#include <boost/noncopyable.hpp>

namespace pgl {
	class server_tls_reload_signal_handler final : boost::noncopyable {
	public:
		using reload_callback = std::function<void()>;

		server_tls_reload_signal_handler(boost::asio::io_context& io_context, reload_callback reload);
		void start();

	private:
		boost::asio::signal_set signals_;
		reload_callback reload_;

		void wait();
	};
}
