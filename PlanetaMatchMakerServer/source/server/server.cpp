#include "server.hpp"

#include "server_thread.hpp"
#include "utilities/log.hpp"

using namespace boost;

namespace pgl {

	server::server(): acceptor_(io_service_), server_setting_() {
		set_output_log_level(log_level::debug);
		server_data_ = std::make_unique<server_data>(std::vector<room_group_name_type>{u8"テスト１", u8"テスト2"});
		server_setting_.max_connections_per_thread = 2;
		server_setting_.time_out_seconds = 300;
		acceptor_.open(get_tcp(ip_version::v4));
		acceptor_.bind(asio::ip::tcp::endpoint(get_tcp(ip_version::v4), 7777));
		acceptor_.listen();
	}

	void server::run() {
		// prevent to stop server when all request are processed
		asio::io_service::work work(io_service_);

		server_thread server_thread(acceptor_, *server_data_, server_setting_);
		server_thread.start();

		io_service_.run();
	}
}
