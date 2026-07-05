#include <fstream>
#include <concepts>
#include <unordered_map>

#include <boost/json.hpp>
#include <boost/lexical_cast.hpp>
#include "nameof.hpp"

#include "minimal_serializer/string_utility.hpp"

#include "server/server_setting.hpp"

#include "authentication/game.hpp"
#include "utilities/env_var.hpp"

using namespace boost;
using namespace minimal_serializer;

namespace pgl {
	const std::string common_section_key = "common";
	const std::string authentication_section_key = "authentication";
	const std::string log_section_key = "log";
	const std::string connection_test_section_key = "connection_test";
	const std::string tls_section_key = "tls";

	server_setting_error::server_setting_error(const std::string& message): std::runtime_error(message) {}
	std::string server_setting_error::message() const { return what(); }

	std::ostream& operator<<(std::ostream& os, const server_setting_error& error) {
		os << error.message();
		return os;
	}

	server_tls_mode string_to_server_tls_mode(const std::string& str) {
		const static std::unordered_map<std::string, server_tls_mode> map{
			{std::string(nameof::nameof_enum(server_tls_mode::plain)), server_tls_mode::plain},
			{std::string(nameof::nameof_enum(server_tls_mode::tls)), server_tls_mode::tls},
		};
		return map.at(str);
	}

	std::ostream& operator<<(std::ostream& os, const server_tls_mode mode) {
		const auto mode_name = nameof::nameof_enum(mode);
		os << (mode_name.empty() ? "invalid" : std::string(mode_name));
		return os;
	}

	template <typename T>
	T extract_with_default(const json::object& obj, const std::string& key, const T& default_value) {
		const auto* value = obj.if_contains(key);
		if (value == nullptr) { return default_value; }
		return json::value_to<T>(*value);
	}

	// std::u8string is not available in ptree with error "array required", so use std::string then convert to std::u8string (boost library in 1.78.0).
	template <>
	std::u8string extract_with_default<std::u8string>(const json::object& obj, const std::string& key,
		const std::u8string& default_value) {
		std::string value(reinterpret_cast<const char*>(default_value.c_str()));
		value = extract_with_default<std::string>(obj, key, value);
		return {reinterpret_cast<const char8_t*>(value.c_str())};
	}

	// std::filesystem::path is not available in ptree, so use std::string then convert to std::filesystem::path (boost library in 1.78.0).
	template <>
	std::filesystem::path extract_with_default<std::filesystem::path>(const json::object& obj, const std::string& key,
		const std::filesystem::path& default_value) {
		std::string value(default_value.string());
		value = extract_with_default<std::string>(obj, key, value);
		return value;
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

	template <typename T>
	void validate_str_length(const std::string& target, const T& v, const std::size_t& min,
		const std::size_t& max) {
		if (v.length() < min) {
			throw server_setting_error(generate_string(target, " is ", v, " but the length of it must be in range ",
				min, "-", max, "."));
		}

		if (v.length() > max) {
			throw server_setting_error(generate_string(target, " is ", v, " but the length of it must be in range ",
				min, "-", max, "."));
		}
	}

	ip_version tag_invoke(json::value_to_tag<ip_version>, const json::value& jv) {
		return string_to_ip_version(json::value_to<std::string>(jv));
	}

	server_tls_mode tag_invoke(json::value_to_tag<server_tls_mode>, const json::value& jv) {
		return string_to_server_tls_mode(json::value_to<std::string>(jv));
	}

	server_common_setting tag_invoke(json::value_to_tag<server_common_setting>, const json::value& jv) {
		const auto* obj = jv.if_object();
		if (obj == nullptr) {
			throw server_setting_error(generate_string("\"", common_section_key, "\" must be object."));
		}

		server_common_setting s;
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
		log(log_level::info, NAMEOF(setting.time_out_seconds), ": ", setting.time_out_seconds);
		log(log_level::info, NAMEOF(setting.ip_version), ": ", setting.ip_version);
		log(log_level::info, NAMEOF(setting.port), ": ", setting.port);
		log(log_level::info, NAMEOF(setting.max_connection_per_thread), ": ", setting.max_connection_per_thread);
		log(log_level::info, NAMEOF(setting.thread), ": ", setting.thread);
		log(log_level::info, NAMEOF(setting.max_room_count), ": ", setting.max_room_count);
		log(log_level::info, NAMEOF(setting.max_player_per_room), ": ", setting.max_player_per_room);
	}

	server_authentication_setting tag_invoke(json::value_to_tag<server_authentication_setting>, const json::value& jv) {
		const auto* obj = jv.if_object();
		if (obj == nullptr) {
			throw server_setting_error(generate_string("\"", authentication_section_key, "\" must be object."));
		}

		server_authentication_setting s;
		EXTRACT_WITH_DEFAULT(*obj, s, std::u8string, game_id);
		EXTRACT_WITH_DEFAULT(*obj, s, bool, enable_game_version_check);
		EXTRACT_WITH_DEFAULT(*obj, s, std::u8string, game_version);
		return s;
	}

	void validate_authentication_setting(const server_authentication_setting& setting) {
		validate_str_length(authentication_section_key + ".game_id", setting.game_id, 1, game_id_bytes);

		if (setting.enable_game_version_check) {
			validate_str_length(authentication_section_key + ".game_version", setting.game_version, 1,
				game_version_bytes);
		}
	}

	void output_authentication_setting_to_log(const server_authentication_setting& setting) {
		log(log_level::info, "--------Authentication--------");
		log(log_level::info, NAMEOF(setting.game_id), ": ", setting.game_id);
		log(log_level::info, NAMEOF(setting.enable_game_version_check), ": ", setting.enable_game_version_check);
		log(log_level::info, NAMEOF(setting.game_version), ": ", setting.game_version);
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
		EXTRACT_WITH_DEFAULT(*obj, s, std::filesystem::path, file_log_path);
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

	server_tls_setting tag_invoke(json::value_to_tag<server_tls_setting>, const json::value& jv) {
		const auto* obj = jv.if_object();
		if (obj == nullptr) {
			throw server_setting_error(generate_string("\"", tls_section_key, "\" must be object."));
		}
		server_tls_setting s;
		EXTRACT_WITH_DEFAULT(*obj, s, server_tls_mode, mode);
		EXTRACT_WITH_DEFAULT(*obj, s, std::filesystem::path, certificate_path);
		EXTRACT_WITH_DEFAULT(*obj, s, std::filesystem::path, private_key_path);
		return s;
	}

	void validate_tls_setting(const server_tls_setting& setting) {
		switch (setting.mode) {
			case server_tls_mode::plain:
				return;
			case server_tls_mode::tls:
				if (setting.certificate_path.empty()) {
					throw server_setting_error(tls_section_key + ".certificate_path must not be empty when tls.mode is tls.");
				}
				if (setting.private_key_path.empty()) {
					throw server_setting_error(tls_section_key + ".private_key_path must not be empty when tls.mode is tls.");
				}
				return;
			default:
				throw server_setting_error(generate_string(tls_section_key, ".mode is invalid."));
		}
	}

	void output_tls_setting_to_log(const server_tls_setting& setting) {
		log(log_level::info, "--------TLS--------");
		log(log_level::info, NAMEOF(setting.mode), ": ", setting.mode);
		log(log_level::info, NAMEOF(setting.certificate_path), ": ", setting.certificate_path);
		log(log_level::info, NAMEOF(setting.private_key_path), ": ", setting.private_key_path);
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

			if (const auto* authentication_section = obj->if_contains(authentication_section_key);
				authentication_section != nullptr) {
				authentication = json::value_to<server_authentication_setting>(*authentication_section);
			}
			validate_authentication_setting(authentication);

			if (const auto* log_section = obj->if_contains(log_section_key); log_section != nullptr) {
				log = json::value_to<server_log_setting>(*log_section);
			}
			validate_log_setting(log);

			if (const auto* connection_test_section = obj->if_contains(connection_test_section_key);
				connection_test_section != nullptr) {
				connection_test = json::value_to<server_connection_test_setting>(*connection_test_section);
			}
			validate_connection_test_setting(connection_test);

			if (const auto* tls_section = obj->if_contains(tls_section_key); tls_section != nullptr) {
				tls = json::value_to<server_tls_setting>(*tls_section);
			}
			validate_tls_setting(tls);
		}
		catch (const std::exception& e) {
			throw server_setting_error(generate_string("Failed to load the file: ", e.what()));
		}
	}

	template <>
	bool get_env_var<ip_version>(const std::string& var_name, ip_version& dest) {
		std::string str;
		if (!get_env_var(var_name, str)) { return false; }
		try { dest = string_to_ip_version(str); }
		catch (const std::out_of_range& e) {
			throw server_setting_error(generate_string("The environment variable \"", var_name, "=", str,
				"\" is not convertible to ip_version (", e.what(), ")."));
		}

		return true;
	}

	template <>
	bool get_env_var<log_level>(const std::string& var_name, log_level& dest) {
		std::string str;
		if (!get_env_var(var_name, str)) { return false; }
		try { dest = string_to_log_level(str); }
		catch (const std::out_of_range& e) {
			throw server_setting_error(generate_string("The environment variable \"", var_name, "=", str,
				"\" is not convertible to log_level (", e.what(), ")."));
		}

		return true;
	}

	template <>
	bool get_env_var<server_tls_mode>(const std::string& var_name, server_tls_mode& dest) {
		std::string str;
		if (!get_env_var(var_name, str)) { return false; }
		try { dest = string_to_server_tls_mode(str); }
		catch (const std::out_of_range& e) {
			throw server_setting_error(generate_string("The environment variable \"", var_name, "=", str,
				"\" is not convertible to server_tls_mode (", e.what(), ")."));
		}

		return true;
	}

	void server_setting::load_from_env_var() {
		get_env_var("PMMS_COMMON_TIME_OUT_SECONDS", common.time_out_seconds);
		get_env_var<ip_version>("PMMS_COMMON_IP_VERSION", common.ip_version);
		get_env_var("PMMS_COMMON_PORT", common.port);
		get_env_var("PMMS_COMMON_MAX_CONNECTION_PER_THREAD", common.max_connection_per_thread);
		get_env_var("PMMS_COMMON_MAX_THREAD", common.thread);
		get_env_var("PMMS_COMMON_MAX_ROOM_COUNT", common.max_room_count);
		get_env_var("PMMS_COMMON_MAX_PLAYER_PER_ROOM", common.max_player_per_room);
		validate_common_setting(common);

		get_env_var("PMMS_AUTHENTICATION_GAME_ID", authentication.game_id);
		get_env_var("PMMS_AUTHENTICATION_ENABLE_GAME_VERSION_CHECK", authentication.enable_game_version_check);
		get_env_var("PMMS_AUTHENTICATION_GAME_VERSION", authentication.game_version);
		validate_authentication_setting(authentication);

		get_env_var("PMMS_LOG_ENABLE_CONSOLE_LOG", log.enable_console_log);
		get_env_var<log_level>("PMMS_LOG_CONSOLE_LOG_LEVEL", log.console_log_level);
		get_env_var("PMMS_LOG_ENABLE_FILE_LOG", log.enable_file_log);
		get_env_var<log_level>("PMMS_LOG_FILE_LOG_LEVEL", log.file_log_level);
		get_env_var("PMMS_LOG_FILE_LOG_PATH", log.file_log_path);
		validate_log_setting(log);

		get_env_var("PMMS_CONNECTION_TEST_CONNECTION_CHECK_TCP_TIME_OUT_SECONDS",
			connection_test.connection_check_tcp_time_out_seconds);
		get_env_var("PMMS_CONNECTION_TEST_CONNECTION_CHECK_UDP_TIME_OUT_SECONDS",
			connection_test.connection_check_udp_time_out_seconds);
		get_env_var("PMMS_CONNECTION_TEST_CONNECTION_CHECK_UDP_TRY_COUNT",
			connection_test.connection_check_udp_try_count);
		validate_connection_test_setting(connection_test);

		get_env_var<server_tls_mode>("PMMS_TLS_MODE", tls.mode);
		get_env_var("PMMS_TLS_CERTIFICATE_PATH", tls.certificate_path);
		get_env_var("PMMS_TLS_PRIVATE_KEY_PATH", tls.private_key_path);
		validate_tls_setting(tls);
	}


	void server_setting::output_to_log() const {
		pgl::log(log_level::info, "================Server Setting================");
		output_common_setting_to_log(common);
		output_authentication_setting_to_log(authentication);
		output_log_setting_to_log(log);
		output_connection_test_setting_to_log(connection_test);
		output_tls_setting_to_log(tls);
		pgl::log(log_level::info, "==============================================");
	}
}
