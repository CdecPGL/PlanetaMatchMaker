#include "client_connection.hpp"

using namespace boost;

namespace pgl {
	client_connection::client_connection(asio::any_io_executor executor, asio::ssl::context& ssl_context):
		ssl_context_(ssl_context), socket_(std::move(executor)) { reset(mode_); }

	void client_connection::reset(const server_tls_mode mode) {
		boost::system::error_code ignored_error;
		close(ignored_error);

		tls_stream_.reset();
		mode_ = mode;
	}

	server_tls_mode client_connection::mode() const { return mode_; }

	bool client_connection::is_tls() const { return mode_ == server_tls_mode::tls; }

	asio::ip::tcp::socket& client_connection::socket() { return socket_; }

	asio::any_io_executor client_connection::get_executor() { return socket().get_executor(); }

	asio::ip::tcp::endpoint client_connection::remote_endpoint() { return socket().remote_endpoint(); }

	asio::ip::tcp::endpoint client_connection::local_endpoint() { return socket().local_endpoint(); }

	void client_connection::async_handshake(const asio::yield_context yield) {
		if (is_tls()) {
			tls_stream_.emplace(socket_, ssl_context_);
			tls_stream_->async_handshake(asio::ssl::stream_base::server, yield);
		}
	}

	void client_connection::cancel(boost::system::error_code& error_code) { socket().cancel(error_code); }

	void client_connection::close(boost::system::error_code& error_code) {
		tls_stream_.reset();
		socket_.close(error_code);
	}
}
