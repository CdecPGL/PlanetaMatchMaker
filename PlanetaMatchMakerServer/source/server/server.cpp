#include <boost/thread.hpp>

#include "server.hpp"

#include "server_thread.hpp"
#include "logger/log.hpp"

using namespace boost;

namespace pgl {

	server::server(std::unique_ptr<server_setting>&& setting): acceptor_(io_service_),
		server_setting_(std::move(setting)) {
		// Setup server data
		server_data_ = std::make_unique<server_data>();

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
		asio::io_service::work work(io_service_);

		log(log_level::info, "Start ", server_setting_->common.thread, " threads.");

		thread_group thread_group;
		for (auto i = 0u; i < server_setting_->common.thread; ++i) {
			thread_group.create_thread([&]() {
				server_thread server_thread(acceptor_, *server_data_, *server_setting_);
				server_thread.start();
				io_service_.run();
			});
		}

		thread_group.join_all();
	}
}
