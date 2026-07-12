#pragma once

#include <cstdint>
#include <filesystem>
#include <iosfwd>
#include <stdexcept>
#include <string>
#include <vector>

#include "authentication/authentication_method.hpp"
#include "logger/log.hpp"
#include "network/network_layer.hpp"

namespace pgl {
	enum class server_tls_mode : uint8_t { plain, tls };

	server_tls_mode string_to_server_tls_mode(const std::string& str);

	class server_setting_error final : public std::runtime_error {
	public:
		explicit server_setting_error(const std::string& message);
		[[nodiscard]] std::string message() const;
	};

	std::ostream& operator <<(std::ostream& os, const server_setting_error& error);

	struct server_common_setting final {
		uint16_t time_out_seconds = 300;
		ip_version ip_version = ip_version::v4;
		uint16_t port = 57000;
		uint16_t max_connection_per_thread = 1000;
		uint16_t thread = 1;
		uint16_t max_room_count = 1000;
		uint8_t max_player_per_room = 16;
	};

	struct server_authentication_setting final {
		std::u8string game_id;
		bool enable_game_version_check = false;
		std::u8string game_version;
		uint32_t max_credential_bytes = 16 * 1024;
		uint16_t timeout_seconds = 5;
		uint16_t clock_skew_seconds = 60;
		bool allow_plain_connections = false;
		bool allow_plain_external_service_connections = false;

		struct steam_setting final {
			bool enabled = false;
			uint32_t app_id = 0;
			std::string publisher_key;
			std::string identity;
			std::string authenticate_user_ticket_url =
				"https://partner.steam-api.com/ISteamUserAuth/AuthenticateUserTicket/v1/";
			std::string check_app_ownership_url =
				"https://partner.steam-api.com/ISteamUser/CheckAppOwnership/v4/";
		} steam;

		struct oidc_setting final {
			bool enabled = false;
			std::string issuer;
			std::string audience;
			std::vector<std::string> algorithms = {"RS256", "RS384", "RS512"};
			std::string discovery_url;
			std::string jwks_url;
			std::string jwks;
			uint32_t jwks_cache_seconds = 3600;
		} oidc;

		[[nodiscard]] bool is_method_enabled(authentication_method method) const;
	};

	struct server_log_setting final {
		bool enable_console_log = true;
		log_level console_log_level = log_level::info;
		bool enable_file_log = true;
		log_level file_log_level = log_level::info;
		std::filesystem::path file_log_path;
	};

	struct server_connection_test_setting final {
		uint16_t connection_check_tcp_time_out_seconds = 5;
		uint16_t connection_check_udp_time_out_seconds = 3;
		uint8_t connection_check_udp_try_count = 3;
	};

	struct server_tls_setting final {
		server_tls_mode mode = server_tls_mode::tls;
		std::filesystem::path certificate_path;
		std::filesystem::path private_key_path;
		bool reload_on_sighup = false;
	};

	// This class need not be thread safe because used for only read access.
	struct server_setting final {
		server_setting() = default;

		server_common_setting common;
		server_authentication_setting authentication;
		server_log_setting log;
		server_connection_test_setting connection_test;
		server_tls_setting tls;

		/**
		 * Load server setting from JSON file.
		 * Settings which is not in json file will not be updated.
		 *
		 * @param file_path A path of setting file.
		 * @exception server_setting_error Loading setting file is failed or the content of the file is invalid.
		 */
		void load_from_json_file(const std::filesystem::path& file_path);

		/**
		 * Load server setting from environment variable.
		 * Settings which is not in environment variable will not be updated.
		 *
		 * @exception server_setting_error Some environment variables are invalid.
		 */
		void load_from_env_var();

		/**
		 * Output content of setting to stdout.
		 */
		void output_to_log() const;
	};
}
