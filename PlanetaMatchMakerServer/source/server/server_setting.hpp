#pragma once

#include <cstdint>
#include <vector>
#include <filesystem>

#include "logger/log.hpp"
#include "network/network_layer.hpp"

namespace pgl {
	struct server_common_setting final {
		bool enable_session_key_check = true;
		uint32_t time_out_seconds = 300;
		ip_version ip_version = ip_version::v4;
		uint16_t port = 57000;
		uint32_t max_connection_per_thread = 1000;
		uint32_t thread = 1;
		uint16_t max_room_count = 1000;
		uint8_t max_player_per_room = 16;
	};

	struct server_log_setting final {
		bool enable_console_log = true;
		log_level console_log_level = log_level::info;
		bool enable_file_log = true;
		log_level file_log_level = log_level::info;
		std::string file_log_path = "";
	};

	struct server_connection_test_setting final {
		uint32_t connection_check_tcp_time_out_seconds = 5;
		uint32_t connection_check_udp_time_out_seconds = 3;
		uint8_t connection_check_udp_try_count = 3;
	};
	
	// This class need not be thread safe because used for only read access.
	struct server_setting final {
		server_setting() = default;

		server_common_setting common;
		server_log_setting log;
		server_connection_test_setting connection_test;
		
		bool load_from_setting_file(const std::filesystem::path& file_path);
		void output_to_log() const;
	};
}
