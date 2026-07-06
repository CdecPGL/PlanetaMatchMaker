#pragma once

#include <memory>
#include <mutex>

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/noncopyable.hpp>

#include "./server_setting.hpp"
#include "./server_data.hpp"
#include "./server_tls_context.hpp"

namespace pgl {
	class server final : boost::noncopyable {
	public:
		explicit server(std::unique_ptr<server_setting>&& setting);
		void run();
	private:
		boost::asio::io_context io_service_;
		server_tls_context tls_context_;
		boost::asio::ip::tcp::acceptor acceptor_;
		std::mutex acceptor_mutex_;
		std::unique_ptr<server_data> server_data_;
		std::unique_ptr<server_setting> server_setting_;

		void reload_tls_context();
		void try_reload_tls_context() noexcept;
	};
}
