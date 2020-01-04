#include <boost/thread.hpp>

#include "server.hpp"

#include "server_thread.hpp"
#include "logger/log.hpp"

using namespace boost;

namespace pgl {

	server::server(const std::filesystem::path& setting_file_path): acceptor_(io_service_) {
		// Load setting
		server_setting_.load_from_setting_file(setting_file_path);

		// Setup log
		set_output_log_level(server_setting_.log_level);

		// Output setting
		log(log_level::info, "Server setting is loaded.");
		server_setting_.output_to_log();

		// Setup server data
		std::vector<room_group_name_type> room_group_name_list(server_setting_.room_group_list.size());
		std::transform(server_setting_.room_group_list.begin(), server_setting_.room_group_list.end(),
			room_group_name_list.begin(), [](const std::string& str) {
				return room_group_name_type(str);
			});
		server_data_ = std::make_unique<server_data>(std::move(room_group_name_list));

		// Setup acceptor
		const auto tcp = get_tcp(server_setting_.ip_version);
		acceptor_.open(tcp);
		acceptor_.bind(asio::ip::tcp::endpoint(tcp, server_setting_.port));
		acceptor_.listen();
	}

	void server::run() {
		// prevent to stop server when all request are processed
		asio::io_service::work work(io_service_);

		log(log_level::info, "Start ", server_setting_.thread, " threads.");

		thread_group thread_group;
		for (auto i = 0u; i < server_setting_.thread; ++i) {
			thread_group.create_thread([&]() {
				server_thread server_thread(acceptor_, *server_data_, server_setting_);
				server_thread.start();
				io_service_.run();
			});
		}

		thread_group.join_all();
	}
}
