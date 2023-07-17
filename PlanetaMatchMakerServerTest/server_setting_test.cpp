#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>
#include <boost/json.hpp>
#include <boost/lexical_cast.hpp>

#include <fstream>

#include "../PlanetaMatchMakerServer/source/server/server_setting.hpp"

using namespace boost;
using namespace pgl;

bool set_env_var(const std::string& var_name, const std::string& value) {
#ifdef _MSC_VER
	return _putenv((var_name + "=" + lexical_cast<std::string>(value)).c_str()) == 0;
#else
	return setenv(var_name.c_str(), value.c_str(), 1) == 0;
#endif
}

bool unset_env_var(const std::string& var_name) {
#ifdef _MSC_VER
	return _putenv((var_name + "=").c_str()) == 0;
#else
	return unsetenv(var_name.c_str()) == 0;
#endif
}

const json::value required_setting = {
	{
		"authentication", {
			{"game_id", "test"},
		}
	}
};

json::value merge_setting(const json::value& base, const json::value& extra) {
	auto merged = base;
	for (auto&& extra_section_pair : extra.as_object()) {
		auto& merged_object = merged.as_object();
		auto merged_section_object = merged_object.contains(extra_section_pair.key())
			                             ? merged_object[extra_section_pair.key()].as_object()
			                             : json::object();
		for (auto&& extra_item_pair : extra_section_pair.value().as_object()) {
			merged_section_object.insert_or_assign(extra_item_pair.key(), extra_item_pair.value());
		}
		merged_object.insert_or_assign(extra_section_pair.key(), std::move(merged_section_object));
	}
	return merged;
}

/**
 * @brief create setting with required setting and extra setting
 * @param extra extra setting. setting exists in both required setting and extra setting is overwritten by extra setting.
 * @return setting
 */
json::value create_setting(const json::value& extra) { return merge_setting(required_setting, extra); }

template <typename T>
void set_typed_env_var(const std::string& name, const T& value) {
	if (!set_env_var(name, lexical_cast<std::string>(value))) { BOOST_FAIL("Failed to set envionment variable"); }
}

template <>
void set_typed_env_var<bool>(const std::string& name, const bool& value) {
	set_typed_env_var(name, value ? "true" : "false");
}

/**
 * @brief set required setting to environment variable
 */
void set_required_setting_env_var() {
	set_typed_env_var("PMMS_AUTHENTICATION_GAME_ID", "test");
}

struct setting_file_fixture {
	setting_file_fixture(): setting_path(std::filesystem::temp_directory_path() / "pmms_test_setting.json") { }
	virtual ~setting_file_fixture() { if (exists(setting_path)) { remove(setting_path); } }

	void create_setting_file(const json::value& value) const {
		std::ofstream setting_stream(setting_path);
		setting_stream << serialize(value);
	}

	std::filesystem::path setting_path;
};

struct env_var_fixture {
	env_var_fixture() { delete_all_pmms_env_vars(); }
	virtual ~env_var_fixture() { delete_all_pmms_env_vars(); }

private:
	static void delete_all_pmms_env_vars() {
		std::vector<std::string> remove_env_list;
		for (char** env_ptr = environ; const auto e = *env_ptr; ++env_ptr) {
			const std::string env(e);
			const auto idx = env.find_first_of(R"(=)");
			if (idx == std::string::npos) { continue; }
			if (const auto name = env.substr(0, idx); name.starts_with("PMMS_")) { remove_env_list.push_back(name); }
		}

		std::ranges::for_each(remove_env_list, [](const std::string& n) { unset_env_var(n); });
	}
};

BOOST_AUTO_TEST_SUITE(server_setting_test)

	BOOST_FIXTURE_TEST_CASE(load_from_json_file_all, setting_file_fixture) {
		// set up
		const json::value test_data = {
			{
				"common", {
					{"enable_session_key_check", false},
					{"time_out_seconds", 100},
					{"ip_version", "v6"},
					{"port", 12345},
					{"max_connection_per_thread", 500},
					{"thread", 400},
					{"max_room_count", 300},
					{"max_player_per_room", 200}
				}
			},
			{
				"authentication", {
					{"game_id", "test"},
					{"enable_game_version_check", true},
					{"game_version", "1.0.0"},
				}
			},
			{
				"log", {
					{"enable_console_log", false},
					{"console_log_level", "warning"},
					{"enable_file_log", false},
					{"file_log_level", "error"},
					{"file_log_path", "test_path"}
				}
			},
			{
				"connection_test", {
					{"connection_check_tcp_time_out_seconds", 10},
					{"connection_check_udp_time_out_seconds", 20},
					{"connection_check_udp_try_count", 30}
				}
			}
		};
		create_setting_file(test_data);

		// exercise
		server_setting setting;
		setting.load_from_json_file(setting_path);

		// verify
		BOOST_CHECK_EQUAL(setting.common.time_out_seconds, 100);
		BOOST_CHECK(setting.common.ip_version == ip_version::v6);
		BOOST_CHECK_EQUAL(setting.common.port, 12345);
		BOOST_CHECK_EQUAL(setting.common.max_connection_per_thread, 500);
		BOOST_CHECK_EQUAL(setting.common.thread, 400);
		BOOST_CHECK_EQUAL(setting.common.max_room_count, 300);
		BOOST_CHECK_EQUAL(setting.common.max_player_per_room, 200);
		// Cannot use BOOST_CHECK_EQUAL because it does not support char8_t
		BOOST_CHECK(setting.authentication.game_id == u8"test");
		BOOST_CHECK_EQUAL(setting.authentication.enable_game_version_check, true);
		// Cannot use BOOST_CHECK_EQUAL because it does not support char8_t
		BOOST_CHECK(setting.authentication.game_version == u8"1.0.0");
		BOOST_CHECK_EQUAL(setting.log.enable_console_log, false);
		BOOST_CHECK(setting.log.console_log_level == log_level::warning);
		BOOST_CHECK_EQUAL(setting.log.enable_file_log, false);
		BOOST_CHECK(setting.log.file_log_level == log_level::error);
		BOOST_CHECK_EQUAL(setting.log.file_log_path, "test_path");
		BOOST_CHECK_EQUAL(setting.connection_test.connection_check_tcp_time_out_seconds, 10);
		BOOST_CHECK_EQUAL(setting.connection_test.connection_check_udp_time_out_seconds, 20);
		BOOST_CHECK_EQUAL(setting.connection_test.connection_check_udp_try_count, 30);
	}

	BOOST_FIXTURE_TEST_CASE(load_from_json_file_minimal, setting_file_fixture) {
		// set up
		const auto test_data = required_setting;
		create_setting_file(test_data);

		// exercise
		server_setting setting;
		setting.load_from_json_file(setting_path);

		// verify
		BOOST_CHECK_EQUAL(setting.common.time_out_seconds, 300);
		BOOST_CHECK(setting.common.ip_version == ip_version::v4);
		BOOST_CHECK_EQUAL(setting.common.port, 57000);
		BOOST_CHECK_EQUAL(setting.common.max_connection_per_thread, 1000);
		BOOST_CHECK_EQUAL(setting.common.thread, 1);
		BOOST_CHECK_EQUAL(setting.common.max_room_count, 1000);
		BOOST_CHECK_EQUAL(setting.common.max_player_per_room, 16);
		// Skip check setting.authentication.enable_game_version_check because it it required setting
		BOOST_CHECK_EQUAL(setting.authentication.enable_game_version_check, false);
		// Cannot use BOOST_CHECK_EQUAL because it does not support char8_t
		BOOST_CHECK(setting.authentication.game_version == u8""); // NOLINT(readability-container-size-empty)
		BOOST_CHECK_EQUAL(setting.log.enable_console_log, true);
		BOOST_CHECK(setting.log.console_log_level == log_level::info);
		BOOST_CHECK_EQUAL(setting.log.enable_file_log, true);
		BOOST_CHECK(setting.log.file_log_level == log_level::info);
		BOOST_CHECK_EQUAL(setting.log.file_log_path, "");
		BOOST_CHECK_EQUAL(setting.connection_test.connection_check_tcp_time_out_seconds, 5);
		BOOST_CHECK_EQUAL(setting.connection_test.connection_check_udp_time_out_seconds, 3);
		BOOST_CHECK_EQUAL(setting.connection_test.connection_check_udp_try_count, 3);
	}

	BOOST_DATA_TEST_CASE_F(setting_file_fixture, test_load_from_json_file_time_out_seconds_valid,
		unit_test::data::make({ 1,3600 })) {
		// set up
		const auto test_data = create_setting({
			{
				"common", {
					{"time_out_seconds", sample},
				}
			}
		});
		create_setting_file(test_data);

		// exercise
		server_setting setting;
		setting.load_from_json_file(setting_path);

		// verify
		BOOST_CHECK_EQUAL(setting.common.time_out_seconds, sample);
	}

	BOOST_DATA_TEST_CASE_F(setting_file_fixture, test_load_from_json_file_ip_version_valid,
		unit_test::data::make({ "v4", "v6"})) {
		// set up
		const auto test_data = create_setting({
			{
				"common", {
					{"ip_version", sample},
				}
			}
		});
		create_setting_file(test_data);

		// exercise
		server_setting setting;
		setting.load_from_json_file(setting_path);

		// verify
		BOOST_CHECK(setting.common.ip_version == string_to_ip_version(sample));
	}

	BOOST_DATA_TEST_CASE_F(setting_file_fixture, test_load_from_json_file_port_valid,
		unit_test::data::make({ 0,65535 })) {
		// set up
		const auto test_data = create_setting({
			{
				"common", {
					{"port", sample},
				}
			}
		});
		create_setting_file(test_data);

		// exercise
		server_setting setting;
		setting.load_from_json_file(setting_path);

		// verify
		BOOST_CHECK_EQUAL(setting.common.port, sample);
	}

	BOOST_DATA_TEST_CASE_F(setting_file_fixture, test_load_from_json_file_max_connection_per_thread_valid,
		unit_test::data::make({ 1,65535 })) {
		// set up
		const auto test_data = create_setting({
			{
				"common", {
					{"max_connection_per_thread", sample},
				}
			}
		});
		create_setting_file(test_data);

		// exercise
		server_setting setting;
		setting.load_from_json_file(setting_path);

		// verify
		BOOST_CHECK_EQUAL(setting.common.max_connection_per_thread, sample);
	}

	BOOST_DATA_TEST_CASE_F(setting_file_fixture, test_load_from_json_file_thread_valid,
		unit_test::data::make({ 1,65535 })) {
		// set up
		const auto test_data = create_setting({
			{
				"common", {
					{"thread", sample},
				}
			}
		});
		create_setting_file(test_data);

		// exercise
		server_setting setting;
		setting.load_from_json_file(setting_path);

		// verify
		BOOST_CHECK_EQUAL(setting.common.thread, sample);
	}

	BOOST_DATA_TEST_CASE_F(setting_file_fixture, test_load_from_json_file_max_room_count_valid,
		unit_test::data::make({ 1,65535 })) {
		// set up
		const auto test_data = create_setting({
			{
				"common", {
					{"max_room_count", sample},
				}
			}
		});
		create_setting_file(test_data);

		// exercise
		server_setting setting;
		setting.load_from_json_file(setting_path);

		// verify
		BOOST_CHECK_EQUAL(setting.common.max_room_count, sample);
	}

	BOOST_DATA_TEST_CASE_F(setting_file_fixture, test_load_from_json_file_max_player_per_room_valid,
		unit_test::data::make({ 1,255 })) {
		// set up
		const auto test_data = create_setting({
			{
				"common", {
					{"max_player_per_room", sample},
				}
			}
		});
		create_setting_file(test_data);

		// exercise
		server_setting setting;
		setting.load_from_json_file(setting_path);

		// verify
		BOOST_CHECK_EQUAL(setting.common.max_player_per_room, sample);
	}

	BOOST_DATA_TEST_CASE_F(setting_file_fixture, test_load_from_json_file_game_id_valid,
		unit_test::data::make({ "1","---------24bytes--------"})) {
		// set up
		const auto test_data = create_setting({
			{
				"authentication", {
					{"game_id", sample},
				}
			}
		});
		create_setting_file(test_data);

		// exercise
		server_setting setting;
		setting.load_from_json_file(setting_path);

		// verify
		// Cannot use BOOST_CHECK_EQUAL because it does not support char8_t
		BOOST_CHECK(setting.authentication.game_id == reinterpret_cast<const char8_t*>(sample));
	}

	BOOST_DATA_TEST_CASE_F(setting_file_fixture, test_load_from_json_file_game_version_valid,
		unit_test::data::make({
			std::tuple(false, ""),
			std::tuple(false, "---------24bytes--------"),
			std::tuple(true, "1"),
			std::tuple(true, "---------24bytes--------")
			}), enable_game_version_check, game_version) {
		// set up
		const auto test_data = create_setting({
			{
				"authentication", {
					{"enable_game_version_check", enable_game_version_check},
					{"game_version", game_version},
				}
			}
		});
		create_setting_file(test_data);

		// exercise
		server_setting setting;
		setting.load_from_json_file(setting_path);

		// verify
		// Cannot use BOOST_CHECK_EQUAL because it does not support char8_t
		BOOST_CHECK(setting.authentication.game_version == reinterpret_cast<const char8_t*>(game_version));
	}

	BOOST_DATA_TEST_CASE_F(setting_file_fixture, test_load_from_json_file_console_log_level_valid,
		unit_test::data::make({ "debug", "info", "warning", "error", "fatal"})) {
		// set up
		const auto test_data = create_setting({
			{
				"log", {
					{"console_log_level", sample},
				}
			}
		});
		create_setting_file(test_data);

		// exercise
		server_setting setting;
		setting.load_from_json_file(setting_path);

		// verify
		BOOST_CHECK(setting.log.console_log_level== string_to_log_level(sample));
	}

	BOOST_DATA_TEST_CASE_F(setting_file_fixture, load_from_json_file_log_level_valid,
		unit_test::data::make({ "debug", "info", "warning", "error", "fatal" })) {
		// set up
		const auto test_data = create_setting({
			{
				"log", {
					{"file_log_level", sample},
				}
			}
		});
		create_setting_file(test_data);

		// exercise
		server_setting setting;
		setting.load_from_json_file(setting_path);

		// verify
		BOOST_CHECK(setting.log.file_log_level == string_to_log_level(sample));
	}

	BOOST_DATA_TEST_CASE_F(setting_file_fixture, test_load_from_json_file_connection_check_tcp_time_out_seconds_valid,
		unit_test::data::make({ 1,3600})) {
		// set up
		const auto test_data = create_setting({
			{
				"connection_test", {
					{"connection_check_tcp_time_out_seconds", sample},
				}
			}
		});
		create_setting_file(test_data);

		// exercise
		server_setting setting;
		setting.load_from_json_file(setting_path);

		// verify
		BOOST_CHECK_EQUAL(setting.connection_test.connection_check_tcp_time_out_seconds, sample);
	}

	BOOST_DATA_TEST_CASE_F(setting_file_fixture, test_load_from_json_file_connection_check_udp_time_out_seconds_valid,
		unit_test::data::make({ 1,3600 })) {
		// set up
		const auto test_data = create_setting({
			{
				"connection_test", {
					{"connection_check_udp_time_out_seconds", sample},
				}
			}
		});
		create_setting_file(test_data);

		// exercise
		server_setting setting;
		setting.load_from_json_file(setting_path);

		// verify
		BOOST_CHECK_EQUAL(setting.connection_test.connection_check_udp_time_out_seconds, sample);
	}

	BOOST_DATA_TEST_CASE_F(setting_file_fixture, test_load_from_json_file_connection_check_udp_try_count_valid,
		unit_test::data::make({ 1,100 })) {
		// set up
		const auto test_data = create_setting({
			{
				"connection_test", {
					{"connection_check_udp_try_count", sample},
				}
			}
		});
		create_setting_file(test_data);

		// exercise
		server_setting setting;
		setting.load_from_json_file(setting_path);

		// verify
		BOOST_CHECK_EQUAL(setting.connection_test.connection_check_udp_try_count, sample);
	}

	BOOST_DATA_TEST_CASE_F(setting_file_fixture, test_load_from_json_file_invalid_integer_value,
		unit_test::data::make({
			std::tuple{"common", "time_out_seconds", - 1},
			std::tuple{"common", "time_out_seconds", 65536},
			std::tuple{"common", "port", -1},
			std::tuple{"common", "port", 65536},
			std::tuple{"common", "max_connection_per_thread", 0},
			std::tuple{"common", "max_connection_per_thread", 65536},
			std::tuple{"common", "thread", 0},
			std::tuple{"common", "thread", 65536},
			std::tuple{"common", "max_room_count", 0},
			std::tuple{"common", "max_room_count", 65536},
			std::tuple{"common", "max_player_per_room", 0},
			std::tuple{"common", "max_player_per_room", 256},
			std::tuple{"connection_test", "connection_check_tcp_time_out_seconds", 0},
			std::tuple{"connection_test", "connection_check_tcp_time_out_seconds", 3601},
			std::tuple{"connection_test", "connection_check_udp_time_out_seconds", 0},
			std::tuple{"connection_test", "connection_check_udp_time_out_seconds", 3601},
			std::tuple{"connection_test", "connection_check_udp_try_count", 0},
			std::tuple{"connection_test", "connection_check_udp_try_count", 101},
			}), section, key, value) {
		// set up
		const auto test_data = create_setting({
			{
				section, {
					{key, value},
				}
			}
		});
		create_setting_file(test_data);

		// exercise and verify
		server_setting setting;
		BOOST_CHECK_THROW(setting.load_from_json_file(setting_path), server_setting_error);
	}

	BOOST_DATA_TEST_CASE_F(setting_file_fixture, test_load_from_json_file_invalid_string_value,
		unit_test::data::make({
			std::tuple{"common", "ip_version", "v5"},
			std::tuple{"common", "ip_version", "4"},
			std::tuple{"common", "ip_version", "6"},
			std::tuple{"authentication", "game_id", ""},
			std::tuple{"authentication", "game_id", "---------25bytes---------"},
			std::tuple{"log", "console_log_level", "inf"},
			std::tuple{"log", "file_log_level", "inf"},
			}), section, key, value) {
		// set up
		const auto test_data = create_setting({
			{
				section, {
					{key, value},
				}
			}
		});
		create_setting_file(test_data);

		// exercise and verify
		server_setting setting;
		BOOST_CHECK_THROW(setting.load_from_json_file(setting_path), server_setting_error);
	}

	BOOST_DATA_TEST_CASE_F(setting_file_fixture, test_load_from_json_file_invalid_game_version_when_check_enabled,
		unit_test::data::make({
			"",
			"---------25bytes---------",
			})) {
		// set up
		const auto test_data = create_setting({
			{
				"authentication", {
					{"enable_game_version_check", true},
					{"game_version", sample},
				}
			}
		});
		create_setting_file(test_data);

		// exercise and verify
		server_setting setting;
		BOOST_CHECK_THROW(setting.load_from_json_file(setting_path), server_setting_error);
	}


	BOOST_FIXTURE_TEST_CASE(load_from_env_var_all, env_var_fixture) {
		// set up
		set_typed_env_var("PMMS_COMMON_ENABLE_SESSION_KEY_CHECK", false);
		set_typed_env_var("PMMS_COMMON_TIME_OUT_SECONDS", 100);
		set_typed_env_var("PMMS_COMMON_IP_VERSION", "v6");
		set_typed_env_var("PMMS_COMMON_PORT", 12345);
		set_typed_env_var("PMMS_COMMON_MAX_CONNECTION_PER_THREAD", 500);
		set_typed_env_var("PMMS_COMMON_MAX_THREAD", 400);
		set_typed_env_var("PMMS_COMMON_MAX_ROOM_COUNT", 300);
		set_typed_env_var("PMMS_COMMON_MAX_PLAYER_PER_ROOM", 200);
		set_typed_env_var("PMMS_AUTHENTICATION_GAME_ID", "test");
		set_typed_env_var("PMMS_AUTHENTICATION_ENABLE_GAME_VERSION_CHECK", true);
		set_typed_env_var("PMMS_AUTHENTICATION_GAME_VERSION", "1.0.0");
		set_typed_env_var("PMMS_LOG_ENABLE_CONSOLE_LOG", false);
		set_typed_env_var("PMMS_LOG_CONSOLE_LOG_LEVEL", "warning");
		set_typed_env_var("PMMS_LOG_ENABLE_FILE_LOG", false);
		set_typed_env_var("PMMS_LOG_FILE_LOG_LEVEL", "error");
		set_typed_env_var("PMMS_LOG_FILE_LOG_PATH", "test_path");
		set_typed_env_var("PMMS_CONNECTION_TEST_CONNECTION_CHECK_TCP_TIME_OUT_SECONDS", 10);
		set_typed_env_var("PMMS_CONNECTION_TEST_CONNECTION_CHECK_UDP_TIME_OUT_SECONDS", 20);
		set_typed_env_var("PMMS_CONNECTION_TEST_CONNECTION_CHECK_UDP_TRY_COUNT", 30);

		// exercise
		server_setting setting;
		setting.load_from_env_var();

		// verify
		BOOST_CHECK_EQUAL(setting.common.time_out_seconds, 100);
		BOOST_CHECK(setting.common.ip_version == ip_version::v6);
		BOOST_CHECK_EQUAL(setting.common.port, 12345);
		BOOST_CHECK_EQUAL(setting.common.max_connection_per_thread, 500);
		BOOST_CHECK_EQUAL(setting.common.thread, 400);
		BOOST_CHECK_EQUAL(setting.common.max_room_count, 300);
		BOOST_CHECK_EQUAL(setting.common.max_player_per_room, 200);
		// Cannot use BOOST_CHECK_EQUAL because it does not support char8_t
		BOOST_CHECK(setting.authentication.game_id == u8"test");
		BOOST_CHECK_EQUAL(setting.authentication.enable_game_version_check, true);
		// Cannot use BOOST_CHECK_EQUAL because it does not support char8_t
		BOOST_CHECK(setting.authentication.game_version == u8"1.0.0");
		BOOST_CHECK_EQUAL(setting.log.enable_console_log, false);
		BOOST_CHECK(setting.log.console_log_level == log_level::warning);
		BOOST_CHECK_EQUAL(setting.log.enable_file_log, false);
		BOOST_CHECK(setting.log.file_log_level == log_level::error);
		BOOST_CHECK_EQUAL(setting.log.file_log_path, "test_path");
		BOOST_CHECK_EQUAL(setting.connection_test.connection_check_tcp_time_out_seconds, 10);
		BOOST_CHECK_EQUAL(setting.connection_test.connection_check_udp_time_out_seconds, 20);
		BOOST_CHECK_EQUAL(setting.connection_test.connection_check_udp_try_count, 30);
	}

	BOOST_FIXTURE_TEST_CASE(load_from_env_var_empty, env_var_fixture) {
		// set up
		// exercise
		set_required_setting_env_var();
		server_setting setting;
		setting.load_from_env_var();

		// verify
		BOOST_CHECK_EQUAL(setting.common.time_out_seconds, 300);
		BOOST_CHECK(setting.common.ip_version == ip_version::v4);
		BOOST_CHECK_EQUAL(setting.common.port, 57000);
		BOOST_CHECK_EQUAL(setting.common.max_connection_per_thread, 1000);
		BOOST_CHECK_EQUAL(setting.common.thread, 1);
		BOOST_CHECK_EQUAL(setting.common.max_room_count, 1000);
		BOOST_CHECK_EQUAL(setting.common.max_player_per_room, 16);
		// Skip check setting.authentication.enable_game_version_check because it it required setting
		BOOST_CHECK_EQUAL(setting.authentication.enable_game_version_check, false);
		// Cannot use BOOST_CHECK_EQUAL because it does not support char8_t
		BOOST_CHECK(setting.authentication.game_version == u8""); // NOLINT(readability-container-size-empty)
		BOOST_CHECK_EQUAL(setting.log.enable_console_log, true);
		BOOST_CHECK(setting.log.console_log_level == log_level::info);
		BOOST_CHECK_EQUAL(setting.log.enable_file_log, true);
		BOOST_CHECK(setting.log.file_log_level == log_level::info);
		BOOST_CHECK_EQUAL(setting.log.file_log_path, "");
		BOOST_CHECK_EQUAL(setting.connection_test.connection_check_tcp_time_out_seconds, 5);
		BOOST_CHECK_EQUAL(setting.connection_test.connection_check_udp_time_out_seconds, 3);
		BOOST_CHECK_EQUAL(setting.connection_test.connection_check_udp_try_count, 3);
	}

	// Test only one case for each setting section because exhaustive test for validation is done in test of load_from_json_file
	BOOST_DATA_TEST_CASE_F(setting_file_fixture, test_load_from_env_var_validation_error,
		unit_test::data::make({
			std::tuple{"PMMS_COMMON_TIME_OUT_SECONDS", "0"},
			std::tuple{"PMMS_AUTHENTICATION_GAME_ID", ""},
			std::tuple{"PMMS_LOG_CONSOLE_LOG_LEVEL", "none"},
			std::tuple{"PMMS_CONNECTION_TEST_CONNECTION_CHECK_TCP_TIME_OUT_SECONDS", "0"},
			}), key, value) {
		// set up
		set_required_setting_env_var();
		set_typed_env_var(key, value);

		// exercise and verify
		server_setting setting;
		BOOST_CHECK_THROW(setting.load_from_env_var(), server_setting_error);
	}

BOOST_AUTO_TEST_SUITE_END()
