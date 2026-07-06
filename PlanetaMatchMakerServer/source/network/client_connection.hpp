#pragma once

#include <optional>

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/noncopyable.hpp>

#include "server/server_setting.hpp"

namespace pgl {
	class client_connection final : boost::noncopyable {
	public:
		client_connection(boost::asio::any_io_executor executor, boost::asio::ssl::context& ssl_context);

		void reset(server_tls_mode mode);

		[[nodiscard]] server_tls_mode mode() const;
		[[nodiscard]] bool is_tls() const;

		boost::asio::ip::tcp::socket& socket();
		[[nodiscard]] boost::asio::any_io_executor get_executor();
		[[nodiscard]] boost::asio::ip::tcp::endpoint remote_endpoint();
		[[nodiscard]] boost::asio::ip::tcp::endpoint local_endpoint();

		void async_handshake(boost::asio::yield_context yield);
		void cancel(boost::system::error_code& error_code);
		void close(boost::system::error_code& error_code);

		template <typename ConstBufferSequence>
		void async_write(const ConstBufferSequence& buffers, boost::asio::yield_context yield) {
			if (is_tls()) {
				boost::asio::async_write(*tls_stream_, buffers, yield);
				return;
			}

			boost::asio::async_write(socket_, buffers, yield);
		}

		template <typename MutableBufferSequence, typename CompletionCondition>
		void async_read(const MutableBufferSequence& buffers, CompletionCondition completion_condition,
			boost::asio::yield_context yield) {
			if (is_tls()) {
				boost::asio::async_read(*tls_stream_, buffers, completion_condition, yield);
				return;
			}

			boost::asio::async_read(socket_, buffers, completion_condition, yield);
		}

	private:
		boost::asio::ssl::context& ssl_context_;
		server_tls_mode mode_ = server_tls_mode::tls;
		boost::asio::ip::tcp::socket socket_;
		std::optional<boost::asio::ssl::stream<boost::asio::ip::tcp::socket&>> tls_stream_;
	};
}
