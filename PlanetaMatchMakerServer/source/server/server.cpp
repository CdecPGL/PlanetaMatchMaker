#include <boost/thread.hpp>
#include <exception>
#include <mutex>

#include "server.hpp"

#include "server_thread.hpp"
#include "logger/log.hpp"

using namespace boost;

namespace pgl {

	server::server(std::unique_ptr<server_setting>&& setting): ssl_context_(asio::ssl::context::tls_server),
		acceptor_(io_service_),
		server_setting_(std::move(setting)) {
		// Setup server data
		server_data_ = std::make_unique<server_data>();

		if (server_setting_->tls.mode == server_tls_mode::tls) {
			ssl_context_.set_options(
				asio::ssl::context::default_workarounds |
				asio::ssl::context::no_sslv2 |
				asio::ssl::context::no_sslv3 |
				asio::ssl::context::no_tlsv1 |
				asio::ssl::context::no_tlsv1_1);
			ssl_context_.use_certificate_chain_file(server_setting_->tls.certificate_path.string());
			ssl_context_.use_private_key_file(server_setting_->tls.private_key_path.string(),
				asio::ssl::context::pem);
		}

		// Setup acceptor
		const auto tcp = get_tcp(server_setting_->common.ip_version);
		acceptor_.open(tcp);
		try { acceptor_.bind(asio::ip::tcp::endpoint(tcp, server_setting_->common.port)); }
		catch (system::system_error&) {
			log(log_level::fatal, "Failed to start listening ", server_setting_->common.port, " port.");
			throw;
		}
		acceptor_.listen();
	}

	void server::run() {
		// prevent to stop server when all request are processed
        auto work = asio::make_work_guard(io_service_);

		log(log_level::info, "Start ", server_setting_->common.thread, " threads.");

		std::mutex exception_mutex;
		std::exception_ptr first_exception;
		thread_group thread_group;
		for (auto i = 0u; i < server_setting_->common.thread; ++i) {
			thread_group.create_thread([&]() {
				try {
					server_thread server_thread(acceptor_, acceptor_mutex_, ssl_context_, *server_data_,
						*server_setting_);
					server_thread.start();
					io_service_.run();
				}
				catch (...) {
					{
						std::lock_guard lock(exception_mutex);
						if (!first_exception) { first_exception = std::current_exception(); }
					}
					io_service_.stop();
				}
			});
		}

		thread_group.join_all();
		if (first_exception) { std::rethrow_exception(first_exception); }
	}
}
