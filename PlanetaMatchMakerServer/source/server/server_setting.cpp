#include <iostream>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include "server/server_setting.hpp"

using namespace boost;

namespace pgl {
	void load_common_setting(const property_tree::ptree& section, server_common_setting& setting) {
		setting.enable_session_key_check = section.get("enable_session_key_check", setting.enable_session_key_check);
		setting.time_out_seconds = section.get("time_out_seconds", setting.time_out_seconds);

		const auto ip_version_str = section.get("ip_version", std::string(nameof::nameof_enum(setting.ip_version)));
		try { setting.ip_version = string_to_ip_version(ip_version_str); }
		catch (const std::out_of_range&) { std::cerr << ip_version_str << " is invalid for ip_version." << std::endl; }

		setting.port = section.get("port", setting.port);
		setting.max_connection_per_thread = section.get("max_connection_per_thread", setting.max_connection_per_thread);
		setting.thread = section.get("thread", setting.thread);
		setting.max_room_count = section.get("max_room_per_room_group", setting.max_room_count);
		setting.max_player_per_room = section.get("max_player_per_room", setting.max_player_per_room);
	}

	void validate_common_setting(server_common_setting& setting) { }

	void output_common_setting_to_log(const server_common_setting& setting) {
		log(log_level::info, "--------Common--------");
		log(log_level::info, NAMEOF(setting.enable_session_key_check), ": ", setting.enable_session_key_check);
		log(log_level::info, NAMEOF(setting.time_out_seconds), ": ", setting.time_out_seconds);
		log(log_level::info, NAMEOF(setting.ip_version), ": ", setting.ip_version);
		log(log_level::info, NAMEOF(setting.port), ": ", setting.port);
		log(log_level::info, NAMEOF(setting.max_connection_per_thread), ": ", setting.max_connection_per_thread);
		log(log_level::info, NAMEOF(setting.thread), ": ", setting.thread);
		log(log_level::info, NAMEOF(setting.max_room_count), ": ", setting.max_room_count);
		log(log_level::info, NAMEOF(setting.max_player_per_room), ": ", setting.max_player_per_room);
	}

	void load_log_setting(const property_tree::ptree& section, server_log_setting& setting) {
		setting.enable_console_log = section.get("enable_console_log", setting.enable_console_log);

		const auto console_log_level_str = section.get("console_log_level",
			std::string(nameof::nameof_enum(setting.console_log_level)));
		try { setting.console_log_level = string_to_log_level(console_log_level_str); }
		catch (const std::out_of_range&) {
			std::cerr << console_log_level_str << " is invalid for log_level." << std::endl;
		}

		setting.enable_file_log = section.get("enable_file_log", setting.enable_file_log);

		const auto file_log_level_str = section.get("file_log_level",
			std::string(nameof::nameof_enum(setting.file_log_level)));
		try { setting.file_log_level = string_to_log_level(file_log_level_str); }
		catch (const std::out_of_range&) {
			std::cerr << file_log_level_str << " is invalid for log_level." << std::endl;
		}

		setting.file_log_path = section.get("file_log_path", setting.file_log_path);
	}

	void output_log_setting_to_log(const server_log_setting& setting) {
		log(log_level::info, "--------Log--------");
		log(log_level::info, NAMEOF(setting.enable_console_log), ": ", setting.enable_console_log);
		log(log_level::info, NAMEOF(setting.console_log_level), ": ", setting.console_log_level);
		log(log_level::info, NAMEOF(setting.enable_file_log), ": ", setting.enable_file_log);
		log(log_level::info, NAMEOF(setting.file_log_level), ": ", setting.file_log_level);
		log(log_level::info, NAMEOF(setting.file_log_path), ": ", setting.file_log_path);
	}

	void load_connection_test_setting(const property_tree::ptree& section, server_connection_test_setting& setting) {
		setting.connection_check_tcp_time_out_seconds = section.get("connection_check_tcp_time_out_seconds",
			setting.connection_check_tcp_time_out_seconds);
		setting.connection_check_udp_time_out_seconds = section.get("connection_check_udp_time_out_seconds",
			setting.connection_check_udp_time_out_seconds);
		setting.connection_check_udp_try_count = section.get("connection_check_udp_try_count",
			setting.connection_check_udp_try_count);
	}

	bool server_setting::load_from_setting_file(const std::filesystem::path& file_path) {
		if (!exists(file_path)) {
			std::cerr << "Failed to load a server setting file (" << file_path << ")." << std::endl;
			return false;
		}

		property_tree::ptree ptree;
		read_json(file_path.string(), ptree);

		const auto& common_section = ptree.get_child("common", {});
		load_common_setting(common_section, common);

		const auto& log_section = ptree.get_child("common", {});
		load_log_setting(log_section, log);

		const auto& connection_test_section = ptree.get_child("common", {});
		load_connection_test_setting(connection_test_section, connection_test);

		return true;
	}

	void output_connection_test_setting_to_log(const server_connection_test_setting& setting) {
		log(log_level::info, "--------Log--------");
		log(log_level::info, NAMEOF(setting.connection_check_tcp_time_out_seconds), ": ",
			setting.connection_check_tcp_time_out_seconds);
		log(log_level::info, NAMEOF(setting.connection_check_udp_time_out_seconds), ": ",
			setting.connection_check_udp_time_out_seconds);
		log(log_level::info, NAMEOF(setting.connection_check_udp_try_count), ": ",
			setting.connection_check_udp_try_count);
	}

	void server_setting::output_to_log() const {
		::pgl::log(log_level::info, "================Server Setting================");
		output_common_setting_to_log(common);
		output_log_setting_to_log(log);
		output_connection_test_setting_to_log(connection_test);
		::pgl::log(log_level::info, "==============================================");
	}
}
