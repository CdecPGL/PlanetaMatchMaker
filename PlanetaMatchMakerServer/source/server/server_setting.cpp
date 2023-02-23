#include <fstream>
#include <concepts>
#include <cstdlib>
#include <algorithm>

#include <boost/json.hpp>
#include <boost/lexical_cast.hpp>

#include "minimal_serializer/string_utility.hpp"

#include "server/server_setting.hpp"
#include "utilities/checked_static_cast.hpp"

using namespace boost;
using namespace minimal_serializer;

namespace pgl {
	const std::string common_section_key = "common";
	const std::string log_section_key = "log";
	const std::string connection_test_section_key = "connection_test";

	server_setting_error::server_setting_error(const std::string& message): std::runtime_error(message) {}
	std::string server_setting_error::message() const { return what(); }

	std::ostream& operator<<(std::ostream& os, const server_setting_error& error) {
		os << error.message();
		return os;
	}

	template <typename T>
	T extract_with_default(const json::object& obj, const std::string& key, const T& default_value) {
		const auto* value = obj.if_contains(key);
		if (value == nullptr) { return default_value; }
		return json::value_to<T>(*value);
	}

#define EXTRACT_WITH_DEFAULT(obj, setting, type, key) setting.key = extract_with_default<type>(obj, #key, (setting.key))

	template <std::totally_ordered T, std::convertible_to<T> R>
	void validate_range(const std::string& target, const T& v, const R& min, const R& max) {
		if (v < min) {
			throw server_setting_error(generate_string(target, " is ", v, " but must be in range ", min, "-", max,
				"."));
		}

		if (v > max) {
			throw server_setting_error(generate_string(target, " is ", v, " but must be in range ", min, "-", max,
				"."));
		}
	}


	ip_version tag_invoke(json::value_to_tag<ip_version>, const json::value& jv) {
		return string_to_ip_version(json::value_to<std::string>(jv));
	}

	server_common_setting tag_invoke(json::value_to_tag<server_common_setting>, const json::value& jv) {
		const auto* obj = jv.if_object();
		if (obj == nullptr) {
			throw server_setting_error(generate_string("\"", common_section_key, "\" must be object."));
		}

		server_common_setting s;
		EXTRACT_WITH_DEFAULT(*obj, s, bool, enable_session_key_check);
		EXTRACT_WITH_DEFAULT(*obj, s, uint16_t, time_out_seconds);
		EXTRACT_WITH_DEFAULT(*obj, s, ip_version, ip_version);
		EXTRACT_WITH_DEFAULT(*obj, s, uint16_t, port);
		EXTRACT_WITH_DEFAULT(*obj, s, uint16_t, max_connection_per_thread);
		EXTRACT_WITH_DEFAULT(*obj, s, uint16_t, thread);
		EXTRACT_WITH_DEFAULT(*obj, s, uint16_t, max_room_count);
		EXTRACT_WITH_DEFAULT(*obj, s, uint8_t, max_player_per_room);
		return s;
	}

	void validate_common_setting(const server_common_setting& setting) {
		validate_range(common_section_key + ".time_out_seconds", setting.time_out_seconds, 1, 3600);
		validate_range(common_section_key + ".port", setting.port, 0, 65535);
		validate_range(common_section_key + ".max_connection_per_thread", setting.max_connection_per_thread, 1, 65535);
		validate_range(common_section_key + ".thread", setting.thread, 1, 65535);
		validate_range(common_section_key + ".max_room_count", setting.max_room_count, 1, 65535);
		validate_range(common_section_key + ".max_player_per_room", setting.max_player_per_room, 1, 255);
	}

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

	log_level tag_invoke(json::value_to_tag<log_level>, const json::value& jv) {
		return string_to_log_level(json::value_to<std::string>(jv));
	}

	server_log_setting tag_invoke(json::value_to_tag<server_log_setting>, const json::value& jv) {
		const auto* obj = jv.if_object();
		if (obj == nullptr) {
			throw server_setting_error(generate_string("\"", log_section_key, "\" must be object."));
		}
		server_log_setting s;
		EXTRACT_WITH_DEFAULT(*obj, s, bool, enable_console_log);
		EXTRACT_WITH_DEFAULT(*obj, s, log_level, console_log_level);
		EXTRACT_WITH_DEFAULT(*obj, s, bool, enable_file_log);
		EXTRACT_WITH_DEFAULT(*obj, s, log_level, file_log_level);
		EXTRACT_WITH_DEFAULT(*obj, s, std::string, file_log_path);
		return s;
	}

	void validate_log_setting(const server_log_setting&) { }

	void output_log_setting_to_log(const server_log_setting& setting) {
		log(log_level::info, "--------Log--------");
		log(log_level::info, NAMEOF(setting.enable_console_log), ": ", setting.enable_console_log);
		log(log_level::info, NAMEOF(setting.console_log_level), ": ", setting.console_log_level);
		log(log_level::info, NAMEOF(setting.enable_file_log), ": ", setting.enable_file_log);
		log(log_level::info, NAMEOF(setting.file_log_level), ": ", setting.file_log_level);
		log(log_level::info, NAMEOF(setting.file_log_path), ": ", setting.file_log_path);
	}

	server_connection_test_setting
	tag_invoke(json::value_to_tag<server_connection_test_setting>, const json::value& jv) {
		const auto* obj = jv.if_object();
		if (obj == nullptr) {
			throw server_setting_error(generate_string("\"", connection_test_section_key, "\" must be object."));
		}
		server_connection_test_setting s;
		EXTRACT_WITH_DEFAULT(*obj, s, uint16_t, connection_check_tcp_time_out_seconds);
		EXTRACT_WITH_DEFAULT(*obj, s, uint16_t, connection_check_udp_time_out_seconds);
		EXTRACT_WITH_DEFAULT(*obj, s, uint8_t, connection_check_udp_try_count);
		return s;
	}

	void validate_connection_test_setting(const server_connection_test_setting& setting) {
		validate_range(connection_test_section_key + ".connection_check_tcp_time_out_seconds",
			setting.connection_check_tcp_time_out_seconds, 1, 3600);
		validate_range(connection_test_section_key + ".connection_check_udp_time_out_seconds",
			setting.connection_check_udp_time_out_seconds, 1, 3600);
		validate_range(connection_test_section_key + ".connection_check_udp_try_count",
			setting.connection_check_udp_try_count, 1,
			100);
	}

	void output_connection_test_setting_to_log(const server_connection_test_setting& setting) {
		log(log_level::info, "--------Connection Test--------");
		log(log_level::info, NAMEOF(setting.connection_check_tcp_time_out_seconds), ": ",
			setting.connection_check_tcp_time_out_seconds);
		log(log_level::info, NAMEOF(setting.connection_check_udp_time_out_seconds), ": ",
			setting.connection_check_udp_time_out_seconds);
		log(log_level::info, NAMEOF(setting.connection_check_udp_try_count), ": ",
			setting.connection_check_udp_try_count);
	}

	void server_setting::load_from_json_file(const std::filesystem::path& file_path) {
		if (!exists(file_path)) { throw server_setting_error(generate_string("\"", file_path, "\" does not exist.")); }

		try {
			std::ifstream ifs(file_path);
			const auto setting_string = std::string(std::istreambuf_iterator(ifs), std::istreambuf_iterator<char>());

			auto value = parse(setting_string, json::storage_ptr(), {
				.allow_comments = true, .allow_trailing_commas = true
			});

			const auto* obj = value.if_object();
			if (obj == nullptr) {
				throw server_setting_error(generate_string("The root of setting JSON file must be object."));
			}

			if (const auto* common_section = obj->if_contains(common_section_key); common_section != nullptr) {
				common = json::value_to<server_common_setting>(*common_section);
			}
			validate_common_setting(common);

			if (const auto* log_section = obj->if_contains(log_section_key); log_section != nullptr) {
				log = json::value_to<server_log_setting>(*log_section);
			}
			validate_log_setting(log);

			if (const auto* connection_test_section = obj->if_contains(connection_test_section_key);
				connection_test_section != nullptr) {
				connection_test = json::value_to<server_connection_test_setting>(*connection_test_section);
			}
			validate_connection_test_setting(connection_test);
		}
		catch (const std::exception& e) {
			throw server_setting_error(generate_string("Failed to load the file: ", e.what()));
		}
	}

	template <typename T>
	bool overwrite_by_env_var(T& value, const std::string& var_name) {
		// determine the size of environment variable
		size_t buffer_length;
		if (const auto result = getenv_s(&buffer_length, nullptr, 0, var_name.c_str()); result != 0) {
			throw server_setting_error(generate_string("The environment variable \"", var_name,
				"\" can not be not read (", result, ")"));
		}
		if (buffer_length == 0) {
			// environment variable not found
			return false;
		}

		// read the environment variable
		const auto buffer = std::make_unique<char[]>(buffer_length);
		if (const auto result = getenv_s(&buffer_length, buffer.get(), buffer_length, var_name.c_str()); result != 0) {
			throw server_setting_error(generate_string("The environment variable \"", var_name,
				"\" can not be not read (", result, ")"));
		}

		const std::string value_str(buffer.get());
		try { value = lexical_cast<T>(value_str); }
		catch (const bad_lexical_cast& e) {
			throw server_setting_error(generate_string("The environment variable \"", var_name, "=", value_str,
				"\" is not convertible to ", nameof::nameof_type<T>(), " (", e.what(), ")"));
		}
		return true;
	}

	template <>
	bool overwrite_by_env_var<bool>(bool& value, const std::string& var_name) {
		std::string str;
		if (!overwrite_by_env_var(str, var_name)) { return false; }
		auto lower_str = str;
		std::ranges::transform(lower_str, lower_str.begin(), [](const char c) {
			return static_cast<char>(tolower(c));
		});
		if (lower_str == "true") { value = true; }
		else if (lower_str == "false") { value = false; }
		else {
			throw server_setting_error(generate_string("The environment variable \"", var_name, "=", str,
				"\" is not convertible to bool (true or false)."));
		}

		return true;
	}

	template <>
	bool overwrite_by_env_var<uint8_t>(uint8_t& value, const std::string& var_name) {
		int32_t v;
		if (!overwrite_by_env_var(v, var_name)) { return false; }
		try { value = range_checked_static_cast<uint8_t>(v); }
		catch (const static_cast_range_error& e) {
			throw server_setting_error(generate_string("The environment variable \"", var_name, "=", v,
				"\" is not convertible to uint8_t.", e.what()));
		}

		return true;
	}

	template <>
	bool overwrite_by_env_var<int8_t>(int8_t& value, const std::string& var_name) {
		int32_t v;
		if (!overwrite_by_env_var(v, var_name)) { return false; }
		try { value = range_checked_static_cast<int8_t>(v); }
		catch (const static_cast_range_error& e) {
			throw server_setting_error(generate_string("The environment variable \"", var_name, "=", v,
				"\" is not convertible to uint8_t.", e.what()));
		}

		return true;
	}

	template <>
	bool overwrite_by_env_var<ip_version>(ip_version& value, const std::string& var_name) {
		std::string str;
		if (!overwrite_by_env_var(str, var_name)) { return false; }
		try { value = string_to_ip_version(str); }
		catch (const std::out_of_range& e) {
			throw server_setting_error(generate_string("The environment variable \"", var_name, "=", str,
				"\" is not convertible to ip_version (", e.what(), ")."));
		}

		return true;
	}

	template <>
	bool overwrite_by_env_var<log_level>(log_level& value, const std::string& var_name) {
		std::string str;
		if (!overwrite_by_env_var(str, var_name)) { return false; }
		try { value = string_to_log_level(str); }
		catch (const std::out_of_range& e) {
			throw server_setting_error(generate_string("The environment variable \"", var_name, "=", str,
				"\" is not convertible to log_level (", e.what(), ")."));
		}

		return true;
	}

	void server_setting::load_from_env_var() {
		overwrite_by_env_var(common.enable_session_key_check, "PMMS_COMMON_ENABLE_SESSION_KEY_CHECK");
		overwrite_by_env_var(common.time_out_seconds, "PMMS_COMMON_TIME_OUT_SECONDS");
		overwrite_by_env_var<ip_version>(common.ip_version, "PMMS_COMMON_IP_VERSION");
		overwrite_by_env_var(common.port, "PMMS_COMMON_PORT");
		overwrite_by_env_var(common.max_connection_per_thread, "PMMS_COMMON_MAX_CONNECTION_PER_THREAD");
		overwrite_by_env_var(common.thread, "PMMS_COMMON_MAX_THREAD");
		overwrite_by_env_var(common.max_room_count, "PMMS_COMMON_MAX_ROOM_COUNT");
		overwrite_by_env_var(common.max_player_per_room, "PMMS_COMMON_MAX_PLAYER_PER_ROOM");
		validate_common_setting(common);

		overwrite_by_env_var(log.enable_console_log, "PMMS_LOG_ENABLE_CONSOLE_LOG");
		overwrite_by_env_var<log_level>(log.console_log_level, "PMMS_LOG_CONSOLE_LOG_LEVEL");
		overwrite_by_env_var(log.enable_file_log, "PMMS_LOG_ENABLE_FILE_LOG");
		overwrite_by_env_var<log_level>(log.file_log_level, "PMMS_LOG_FILE_LOG_LEVEL");
		overwrite_by_env_var(log.file_log_path, "PMMS_LOG_FILE_LOG_PATH");
		validate_log_setting(log);

		overwrite_by_env_var(connection_test.connection_check_tcp_time_out_seconds,
			"PMMS_CONNECTION_TEST_CONNECTION_CHECK_TCP_TIME_OUT_SECONDS");
		overwrite_by_env_var(connection_test.connection_check_udp_time_out_seconds,
			"PMMS_CONNECTION_TEST_CONNECTION_CHECK_UDP_TIME_OUT_SECONDS");
		overwrite_by_env_var(connection_test.connection_check_udp_try_count,
			"PMMS_CONNECTION_TEST_CONNECTION_CHECK_UDP_TRY_COUNT");
		validate_connection_test_setting(connection_test);
	}


	void server_setting::output_to_log() const {
		pgl::log(log_level::info, "================Server Setting================");
		output_common_setting_to_log(common);
		output_log_setting_to_log(log);
		output_connection_test_setting_to_log(connection_test);
		pgl::log(log_level::info, "==============================================");
	}
}