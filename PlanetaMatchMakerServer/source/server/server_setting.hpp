#pragma once

#include <cstdint>
#include <vector>
#include <filesystem>

#include "logger/log.hpp"
#include "network/network_layer.hpp"

namespace pgl {
	// This class need not be thread safe because used for only read access.
	struct server_setting final {
		server_setting() = default;

		bool enable_session_key_check = false;
		uint32_t time_out_seconds = 300;
		uint32_t connection_check_time_out_seconds = 10;
		ip_version ip_version = ip_version::v4;
		uint16_t port = 7777;
		uint32_t max_connection_per_thread = 1000;
		uint32_t thread = 1;
		uint8_t max_room_per_room_group = 100;
		std::vector<std::string> room_group_list = {"default"};

		bool enable_console_log = true;
		log_level console_log_level = log_level::info;
		bool enable_file_log = true;
		log_level file_log_level = log_level::info;
		std::string file_log_path = "";

		bool load_from_setting_file(const std::filesystem::path& file_path);
		void output_to_log() const;
	};
}
