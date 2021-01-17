#include <iostream>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/algorithm/string/join.hpp>

#include "server/server_setting.hpp"

using namespace boost;

namespace pgl {

	bool server_setting::load_from_setting_file(const std::filesystem::path& file_path) {
		if (!exists(file_path)) {
			std::cerr << "Failed to load a server setting file (" << file_path << ")." << std::endl;
			return false;
		}

		property_tree::ptree ptree;
		read_json(file_path.string(), ptree);
		enable_session_key_check = ptree.get("enable_session_key_check", enable_session_key_check);
		time_out_seconds = ptree.get("time_out_seconds", time_out_seconds);
		connection_check_time_out_seconds = ptree.get("connection_check_time_out_seconds",
			connection_check_time_out_seconds);
		connection_check_udp_try_count = ptree.get("connection_check_udp_try_count",
			connection_check_udp_try_count);
		const auto ip_version_str = ptree.get("ip_version", std::string(nameof::nameof_enum(ip_version)));
		try { ip_version = string_to_ip_version(ip_version_str); }
		catch (const std::out_of_range&) { std::cerr << ip_version_str << " is invalid for ip_version." << std::endl; }
		port = ptree.get("port", port);
		max_connection_per_thread = ptree.get("max_connection_per_thread", max_connection_per_thread);
		thread = ptree.get("thread", thread);
		max_room_per_room_group = ptree.get("max_room_per_room_group", max_room_per_room_group);

		const auto room_group_list_ptree = ptree.get_child_optional("room_group_list");
		if (room_group_list_ptree) {
			room_group_list.clear();
			room_group_list.reserve(room_group_list_ptree->size());
			for (auto&& child : *room_group_list_ptree) {
				room_group_list.push_back(child.second.get_value<std::string>());
			}
		}

		max_player_per_room = ptree.get("max_player_per_room", max_player_per_room);

		enable_console_log = ptree.get("enable_console_log", enable_console_log);
		const auto console_log_level_str = ptree.get("console_log_level",
			std::string(nameof::nameof_enum(console_log_level)));
		try { console_log_level = string_to_log_level(console_log_level_str); }
		catch (const std::out_of_range&) {
			std::cerr << console_log_level_str << " is invalid for log_level." << std::endl;
		}
		enable_file_log = ptree.get("enable_file_log", enable_file_log);
		const auto file_log_level_str = ptree.get("file_log_level", std::string(nameof::nameof_enum(file_log_level)));
		try { file_log_level = string_to_log_level(file_log_level_str); }
		catch (const std::out_of_range&) {
			std::cerr << file_log_level_str << " is invalid for log_level." << std::endl;
		}
		file_log_path = ptree.get("file_log_path", file_log_path);

		return true;
	}

	void server_setting::output_to_log() const {
		log(log_level::info, "================Server Setting================");
		log(log_level::info, NAMEOF(enable_session_key_check), ": ", enable_session_key_check);
		log(log_level::info, NAMEOF(time_out_seconds), ": ", time_out_seconds);
		log(log_level::info, NAMEOF(connection_check_time_out_seconds), ": ", connection_check_time_out_seconds);
		log(log_level::info, NAMEOF(connection_check_udp_try_count), ": ", connection_check_udp_try_count);
		log(log_level::info, NAMEOF(ip_version), ": ", ip_version);
		log(log_level::info, NAMEOF(port), ": ", port);
		log(log_level::info, NAMEOF(max_connection_per_thread), ": ", max_connection_per_thread);
		log(log_level::info, NAMEOF(thread), ": ", thread);
		log(log_level::info, NAMEOF(max_room_per_room_group), ": ", max_room_per_room_group);
		log(log_level::info, NAMEOF(room_group_list), ": ", room_group_list.size(), "(", join(room_group_list, ","),
			")");
		log(log_level::info, NAMEOF(max_player_per_room), ": ", max_player_per_room);
		log(log_level::info, NAMEOF(enable_console_log), ": ", enable_console_log);
		log(log_level::info, NAMEOF(console_log_level), ": ", console_log_level);
		log(log_level::info, NAMEOF(enable_file_log), ": ", enable_file_log);
		log(log_level::info, NAMEOF(file_log_level), ": ", file_log_level);
		log(log_level::info, NAMEOF(file_log_path), ": ", file_log_path);
		log(log_level::info, "==============================================");
	}
}
