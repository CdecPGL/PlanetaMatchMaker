#include <fstream>
#include <concepts>
#include <limits>
#include <ranges>
#include <sstream>
#include <unordered_map>
#include <unordered_set>

#include <boost/algorithm/string.hpp>
#include <boost/json.hpp>
#include <boost/lexical_cast.hpp>
#include "nameof.hpp"

#include "minimal_serializer/string_utility.hpp"

#include "server/server_setting.hpp"

#include "authentication/game.hpp"
#include "message/messages.hpp"
#include "utilities/env_var.hpp"

using namespace boost;
using namespace minimal_serializer;

namespace pgl {
	const std::string common_section_key = "common";
	const std::string authentication_section_key = "authentication";
	const std::string log_section_key = "log";
	const std::string connection_test_section_key = "connection_test";
	const std::string tls_section_key = "tls";
	const std::filesystem::path default_tls_certificate_file_name = "server.crt";
	const std::filesystem::path default_tls_private_key_file_name = "server.key";

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

	bool server_authentication_setting::accepts_method(const authentication_method requested_method) const {
		return method == requested_method;
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

	template <>
	std::vector<std::string> extract_with_default<std::vector<std::string>>(const json::object& obj,
		const std::string& key, const std::vector<std::string>& default_value) {
		const auto* value = obj.if_contains(key);
		if (value == nullptr) { return default_value; }
		const auto* array = value->if_array();
		if (array == nullptr) {
			throw server_setting_error(generate_string("\"", key, "\" must be array."));
		}

		std::vector<std::string> result;
		result.reserve(array->size());
		for (const auto& item : *array) { result.push_back(json::value_to<std::string>(item)); }
		return result;
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

	void validate_external_authentication_url(const std::string& target, const std::string& url,
		const bool allow_plain_external_service_connections) {
		validate_str_length(target, url, 1, 2048);
		if (url.starts_with("https://")) { return; }
		if (allow_plain_external_service_connections && url.starts_with("http://")) { return; }
		throw server_setting_error(target + (allow_plain_external_service_connections
			? " must use the http or https scheme."
			: " must use https. Set authentication.allow_plain_external_service_connections only for development."));
	}

	ip_version tag_invoke(json::value_to_tag<ip_version>, const json::value& jv) {
		return string_to_ip_version(json::value_to<std::string>(jv));
	}

	server_tls_mode tag_invoke(json::value_to_tag<server_tls_mode>, const json::value& jv) {
		return string_to_server_tls_mode(json::value_to<std::string>(jv));
	}

	authentication_method tag_invoke(json::value_to_tag<authentication_method>, const json::value& jv) {
		return string_to_authentication_method(json::value_to<std::string>(jv));
	}

	server_authentication_setting::steam_setting tag_invoke(
		json::value_to_tag<server_authentication_setting::steam_setting>, const json::value& jv) {
		const auto* obj = jv.if_object();
		if (obj == nullptr) {
			throw server_setting_error(generate_string("\"", authentication_section_key, ".steam\" must be object."));
		}

		server_authentication_setting::steam_setting s;
		EXTRACT_WITH_DEFAULT(*obj, s, uint32_t, app_id);
		EXTRACT_WITH_DEFAULT(*obj, s, std::string, publisher_key);
		EXTRACT_WITH_DEFAULT(*obj, s, std::string, identity);
		EXTRACT_WITH_DEFAULT(*obj, s, std::string, authenticate_user_ticket_url);
		EXTRACT_WITH_DEFAULT(*obj, s, std::string, check_app_ownership_url);
		return s;
	}

	server_authentication_setting::oidc_setting tag_invoke(
		json::value_to_tag<server_authentication_setting::oidc_setting>, const json::value& jv) {
		const auto* obj = jv.if_object();
		if (obj == nullptr) {
			throw server_setting_error(generate_string("\"", authentication_section_key, ".oidc\" must be object."));
		}

		server_authentication_setting::oidc_setting s;
		EXTRACT_WITH_DEFAULT(*obj, s, std::string, issuer);
		EXTRACT_WITH_DEFAULT(*obj, s, std::string, audience);
		EXTRACT_WITH_DEFAULT(*obj, s, std::vector<std::string>, algorithms);
		EXTRACT_WITH_DEFAULT(*obj, s, std::string, discovery_url);
		EXTRACT_WITH_DEFAULT(*obj, s, std::string, jwks_url);
		EXTRACT_WITH_DEFAULT(*obj, s, std::string, jwks);
		EXTRACT_WITH_DEFAULT(*obj, s, uint32_t, jwks_cache_seconds);
		return s;
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
		const auto* method = obj->if_contains("method");
		if (method == nullptr) {
			throw server_setting_error(authentication_section_key + ".method is required.");
		}
		s.method = json::value_to<authentication_method>(*method);
		EXTRACT_WITH_DEFAULT(*obj, s, std::u8string, game_id);
		EXTRACT_WITH_DEFAULT(*obj, s, bool, enable_game_version_check);
		EXTRACT_WITH_DEFAULT(*obj, s, std::u8string, game_version);
		EXTRACT_WITH_DEFAULT(*obj, s, uint32_t, max_credential_bytes);
		EXTRACT_WITH_DEFAULT(*obj, s, uint16_t, timeout_seconds);
		EXTRACT_WITH_DEFAULT(*obj, s, uint16_t, clock_skew_seconds);
		EXTRACT_WITH_DEFAULT(*obj, s, bool, allow_plain_connections);
		EXTRACT_WITH_DEFAULT(*obj, s, bool, allow_plain_external_service_connections);
		if (const auto* steam_section = obj->if_contains("steam"); steam_section != nullptr) {
			s.steam = json::value_to<server_authentication_setting::steam_setting>(*steam_section);
		}
		if (const auto* oidc_section = obj->if_contains("oidc"); oidc_section != nullptr) {
			s.oidc = json::value_to<server_authentication_setting::oidc_setting>(*oidc_section);
		}
		return s;
	}

	void validate_authentication_setting(const server_authentication_setting& setting) {
		validate_str_length(authentication_section_key + ".game_id", setting.game_id, 1, game_id_bytes);
		validate_range(authentication_section_key + ".max_credential_bytes", setting.max_credential_bytes, 1u,
			message_attachment_max_bytes);
		validate_range(authentication_section_key + ".timeout_seconds", setting.timeout_seconds, 1, 3600);
		validate_range(authentication_section_key + ".clock_skew_seconds", setting.clock_skew_seconds, 0, 3600);

		if (setting.enable_game_version_check) {
			validate_str_length(authentication_section_key + ".game_version", setting.game_version, 1,
				game_version_bytes);
		}

		if (setting.method == authentication_method::steam) {
			validate_range(authentication_section_key + ".steam.app_id", setting.steam.app_id, 1u,
				std::numeric_limits<uint32_t>::max());
			validate_str_length(authentication_section_key + ".steam.publisher_key", setting.steam.publisher_key,
				1, 4096);
			validate_external_authentication_url(authentication_section_key + ".steam.authenticate_user_ticket_url",
				setting.steam.authenticate_user_ticket_url, setting.allow_plain_external_service_connections);
			validate_external_authentication_url(authentication_section_key + ".steam.check_app_ownership_url",
				setting.steam.check_app_ownership_url, setting.allow_plain_external_service_connections);
		}

		if (setting.method == authentication_method::oidc) {
			validate_str_length(authentication_section_key + ".oidc.issuer", setting.oidc.issuer, 1, 2048);
			validate_str_length(authentication_section_key + ".oidc.audience", setting.oidc.audience, 1, 2048);
			if (setting.oidc.algorithms.empty()) {
				throw server_setting_error(authentication_section_key + ".oidc.algorithms must not be empty.");
			}
			const static std::unordered_set<std::string> supported_algorithms{"RS256", "RS384", "RS512"};
			for (const auto& algorithm : setting.oidc.algorithms) {
				if (!supported_algorithms.contains(algorithm)) {
					throw server_setting_error(generate_string(authentication_section_key,
						".oidc.algorithms contains unsupported algorithm: ", algorithm, "."));
				}
			}
			if (setting.oidc.discovery_url.empty() && setting.oidc.jwks_url.empty() && setting.oidc.jwks.empty()) {
				throw server_setting_error(authentication_section_key +
					".oidc.discovery_url, jwks_url or jwks must be configured when oidc is enabled.");
			}
			if (!setting.oidc.discovery_url.empty()) {
				validate_external_authentication_url(authentication_section_key + ".oidc.discovery_url",
					setting.oidc.discovery_url, setting.allow_plain_external_service_connections);
			}
			if (!setting.oidc.jwks_url.empty()) {
				validate_external_authentication_url(authentication_section_key + ".oidc.jwks_url",
					setting.oidc.jwks_url, setting.allow_plain_external_service_connections);
			}
			validate_range(authentication_section_key + ".oidc.jwks_cache_seconds", setting.oidc.jwks_cache_seconds,
				0u, std::numeric_limits<uint32_t>::max());
		}
	}

	void output_authentication_setting_to_log(const server_authentication_setting& setting) {
		log(log_level::info, "--------Authentication--------");
		log(log_level::info, NAMEOF(setting.method), ": ", setting.method);
		log(log_level::info, NAMEOF(setting.game_id), ": ", setting.game_id);
		log(log_level::info, NAMEOF(setting.enable_game_version_check), ": ", setting.enable_game_version_check);
		log(log_level::info, NAMEOF(setting.game_version), ": ", setting.game_version);
		log(log_level::info, NAMEOF(setting.max_credential_bytes), ": ", setting.max_credential_bytes);
		log(log_level::info, NAMEOF(setting.timeout_seconds), ": ", setting.timeout_seconds);
		log(log_level::info, NAMEOF(setting.clock_skew_seconds), ": ", setting.clock_skew_seconds);
		log(log_level::info, NAMEOF(setting.allow_plain_connections), ": ", setting.allow_plain_connections);
		log(log_level::info, NAMEOF(setting.allow_plain_external_service_connections), ": ",
			setting.allow_plain_external_service_connections);
		if (setting.method == authentication_method::none) {
			log(log_level::warning,
				"Unauthenticated client connections are enabled. Use only for development.");
		}
		if (setting.allow_plain_external_service_connections) {
			log(log_level::warning,
				"Plain HTTP connections to external authentication services are enabled. Use only for development.");
		}
		log(log_level::info, "steam.app_id: ", setting.steam.app_id);
		log(log_level::info, "steam.publisher_key: ", setting.steam.publisher_key.empty() ? "(not set)" : "(set)");
		log(log_level::info, "steam.identity: ", setting.steam.identity);
		log(log_level::info, "steam.authenticate_user_ticket_url: ", setting.steam.authenticate_user_ticket_url);
		log(log_level::info, "steam.check_app_ownership_url: ", setting.steam.check_app_ownership_url);
		log(log_level::info, "oidc.issuer: ", setting.oidc.issuer);
		log(log_level::info, "oidc.audience: ", setting.oidc.audience);
		log(log_level::info, "oidc.algorithms: ", boost::algorithm::join(setting.oidc.algorithms, ","));
		log(log_level::info, "oidc.discovery_url: ", setting.oidc.discovery_url);
		log(log_level::info, "oidc.jwks_url: ", setting.oidc.jwks_url);
		log(log_level::info, "oidc.jwks: ", setting.oidc.jwks.empty() ? "(not set)" : "(set)");
		log(log_level::info, "oidc.jwks_cache_seconds: ", setting.oidc.jwks_cache_seconds);
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
		EXTRACT_WITH_DEFAULT(*obj, s, bool, reload_on_sighup);
		return s;
	}

	void apply_default_tls_file_paths_for_setting_file(server_tls_setting& setting,
		const std::filesystem::path& setting_file_path) {
		const auto setting_directory = setting_file_path.parent_path();
		if (setting.certificate_path.empty()) {
			setting.certificate_path = setting_directory / default_tls_certificate_file_name;
		}
		if (setting.private_key_path.empty()) {
			setting.private_key_path = setting_directory / default_tls_private_key_file_name;
		}
	}

	server_tls_setting load_tls_setting_from_json_file(const json::object& obj,
		const std::filesystem::path& setting_file_path, const server_tls_setting& default_setting) {
		server_tls_setting s = default_setting;
		apply_default_tls_file_paths_for_setting_file(s, setting_file_path);

		const auto* tls_section = obj.if_contains(tls_section_key);
		if (tls_section == nullptr) { return s; }

		const auto* tls_obj = tls_section->if_object();
		if (tls_obj == nullptr) {
			throw server_setting_error(generate_string("\"", tls_section_key, "\" must be object."));
		}

		EXTRACT_WITH_DEFAULT(*tls_obj, s, server_tls_mode, mode);
		EXTRACT_WITH_DEFAULT(*tls_obj, s, std::filesystem::path, certificate_path);
		EXTRACT_WITH_DEFAULT(*tls_obj, s, std::filesystem::path, private_key_path);
		EXTRACT_WITH_DEFAULT(*tls_obj, s, bool, reload_on_sighup);
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
		log(log_level::info, NAMEOF(setting.reload_on_sighup), ": ", setting.reload_on_sighup);
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

			tls = load_tls_setting_from_json_file(*obj, file_path, tls);
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

	template <>
	bool get_env_var<authentication_method>(const std::string& var_name, authentication_method& dest) {
		std::string str;
		if (!get_env_var(var_name, str)) { return false; }
		try { dest = string_to_authentication_method(str); }
		catch (const std::out_of_range& e) {
			throw server_setting_error(generate_string("The environment variable \"", var_name, "=", str,
				"\" is not convertible to authentication_method (", e.what(), ")."));
		}

		return true;
	}

	template <>
	bool get_env_var<std::vector<std::string>>(const std::string& var_name, std::vector<std::string>& dest) {
		std::string str;
		if (!get_env_var(var_name, str)) { return false; }
		std::vector<std::string> values;
		boost::split(values, str, boost::is_any_of(","));
		std::erase_if(values, [](const std::string& value) { return value.empty(); });
		dest = std::move(values);
		return true;
	}

	void server_setting::load_from_env_var() {
		try {
			get_env_var("PMMS_COMMON_TIME_OUT_SECONDS", common.time_out_seconds);
			get_env_var<ip_version>("PMMS_COMMON_IP_VERSION", common.ip_version);
			get_env_var("PMMS_COMMON_PORT", common.port);
			get_env_var("PMMS_COMMON_MAX_CONNECTION_PER_THREAD", common.max_connection_per_thread);
			get_env_var("PMMS_COMMON_MAX_THREAD", common.thread);
			get_env_var("PMMS_COMMON_MAX_ROOM_COUNT", common.max_room_count);
			get_env_var("PMMS_COMMON_MAX_PLAYER_PER_ROOM", common.max_player_per_room);
			validate_common_setting(common);

			if (!get_env_var<authentication_method>("PMMS_AUTHENTICATION_METHOD", authentication.method)) {
				throw server_setting_error("The environment variable PMMS_AUTHENTICATION_METHOD is required.");
			}
			get_env_var("PMMS_AUTHENTICATION_GAME_ID", authentication.game_id);
			get_env_var("PMMS_AUTHENTICATION_ENABLE_GAME_VERSION_CHECK", authentication.enable_game_version_check);
			get_env_var("PMMS_AUTHENTICATION_GAME_VERSION", authentication.game_version);
			get_env_var("PMMS_AUTHENTICATION_MAX_CREDENTIAL_BYTES", authentication.max_credential_bytes);
			get_env_var("PMMS_AUTHENTICATION_TIMEOUT_SECONDS", authentication.timeout_seconds);
			get_env_var("PMMS_AUTHENTICATION_CLOCK_SKEW_SECONDS", authentication.clock_skew_seconds);
			get_env_var("PMMS_AUTHENTICATION_ALLOW_PLAIN_CONNECTIONS", authentication.allow_plain_connections);
			get_env_var("PMMS_AUTHENTICATION_ALLOW_PLAIN_EXTERNAL_SERVICE_CONNECTIONS",
				authentication.allow_plain_external_service_connections);
			get_env_var("PMMS_AUTHENTICATION_STEAM_APP_ID", authentication.steam.app_id);
			get_env_var("PMMS_AUTHENTICATION_STEAM_PUBLISHER_KEY", authentication.steam.publisher_key);
			get_env_var("PMMS_AUTHENTICATION_STEAM_IDENTITY", authentication.steam.identity);
			get_env_var("PMMS_AUTHENTICATION_STEAM_AUTHENTICATE_USER_TICKET_URL",
				authentication.steam.authenticate_user_ticket_url);
			get_env_var("PMMS_AUTHENTICATION_STEAM_CHECK_APP_OWNERSHIP_URL",
				authentication.steam.check_app_ownership_url);
			get_env_var("PMMS_AUTHENTICATION_OIDC_ISSUER", authentication.oidc.issuer);
			get_env_var("PMMS_AUTHENTICATION_OIDC_AUDIENCE", authentication.oidc.audience);
			get_env_var("PMMS_AUTHENTICATION_OIDC_ALGORITHMS", authentication.oidc.algorithms);
			get_env_var("PMMS_AUTHENTICATION_OIDC_DISCOVERY_URL", authentication.oidc.discovery_url);
			get_env_var("PMMS_AUTHENTICATION_OIDC_JWKS_URL", authentication.oidc.jwks_url);
			get_env_var("PMMS_AUTHENTICATION_OIDC_JWKS", authentication.oidc.jwks);
			get_env_var("PMMS_AUTHENTICATION_OIDC_JWKS_CACHE_SECONDS", authentication.oidc.jwks_cache_seconds);
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
			get_env_var("PMMS_TLS_RELOAD_ON_SIGHUP", tls.reload_on_sighup);
			validate_tls_setting(tls);
		}
		catch (const server_setting_error&) {
			throw;
		}
		catch (const std::exception& e) {
			throw server_setting_error(generate_string("Failed to load environment variables: ", e.what()));
		}
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
