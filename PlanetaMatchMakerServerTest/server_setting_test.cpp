#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>
#include <boost/json.hpp>

#include <fstream>

#include "../PlanetaMatchMakerServer/source/server/server_setting.hpp"

using namespace boost;
using namespace pgl;

struct fixture {
	fixture(): setting_path(std::filesystem::temp_directory_path() / "pmms_test_setting.json") { }
	virtual ~fixture() { if (exists(setting_path)) { remove(setting_path); } }

	void create_setting_file(const json::value& value) const {
		std::ofstream setting_stream(setting_path);
		setting_stream << serialize(value);
	}

	std::filesystem::path setting_path;
};

BOOST_AUTO_TEST_SUITE(server_setting_test)

	BOOST_FIXTURE_TEST_CASE(test_load_from_setting_file_all, fixture) {
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
		setting.load_from_setting_file(setting_path);

		// verify
		BOOST_CHECK_EQUAL(setting.common.enable_session_key_check, false);
		BOOST_CHECK_EQUAL(setting.common.time_out_seconds, 100);
		BOOST_CHECK(setting.common.ip_version == ip_version::v6);
		BOOST_CHECK_EQUAL(setting.common.port, 12345);
		BOOST_CHECK_EQUAL(setting.common.max_connection_per_thread, 500);
		BOOST_CHECK_EQUAL(setting.common.thread, 400);
		BOOST_CHECK_EQUAL(setting.common.max_room_count, 300);
		BOOST_CHECK_EQUAL(setting.common.max_player_per_room, 200);
		BOOST_CHECK_EQUAL(setting.log.enable_console_log, false);
		BOOST_CHECK(setting.log.console_log_level == log_level::warning);
		BOOST_CHECK_EQUAL(setting.log.enable_file_log, false);
		BOOST_CHECK(setting.log.file_log_level == log_level::error);
		BOOST_CHECK_EQUAL(setting.log.file_log_path, "test_path");
		BOOST_CHECK_EQUAL(setting.connection_test.connection_check_tcp_time_out_seconds, 10);
		BOOST_CHECK_EQUAL(setting.connection_test.connection_check_udp_time_out_seconds, 20);
		BOOST_CHECK_EQUAL(setting.connection_test.connection_check_udp_try_count, 30);
	}

	BOOST_FIXTURE_TEST_CASE(test_load_from_setting_file_empty, fixture) {
		// set up
		const json::value test_data = json::object();
		create_setting_file(test_data);

		// exercise
		server_setting setting;
		setting.load_from_setting_file(setting_path);

		// verify
		BOOST_CHECK_EQUAL(setting.common.enable_session_key_check, true);
		BOOST_CHECK_EQUAL(setting.common.time_out_seconds, 300);
		BOOST_CHECK(setting.common.ip_version == ip_version::v4);
		BOOST_CHECK_EQUAL(setting.common.port, 57000);
		BOOST_CHECK_EQUAL(setting.common.max_connection_per_thread, 1000);
		BOOST_CHECK_EQUAL(setting.common.thread, 1);
		BOOST_CHECK_EQUAL(setting.common.max_room_count, 1000);
		BOOST_CHECK_EQUAL(setting.common.max_player_per_room, 16);
		BOOST_CHECK_EQUAL(setting.log.enable_console_log, true);
		BOOST_CHECK(setting.log.console_log_level == log_level::info);
		BOOST_CHECK_EQUAL(setting.log.enable_file_log, true);
		BOOST_CHECK(setting.log.file_log_level == log_level::info);
		BOOST_CHECK_EQUAL(setting.log.file_log_path, "");
		BOOST_CHECK_EQUAL(setting.connection_test.connection_check_tcp_time_out_seconds, 5);
		BOOST_CHECK_EQUAL(setting.connection_test.connection_check_udp_time_out_seconds, 3);
		BOOST_CHECK_EQUAL(setting.connection_test.connection_check_udp_try_count, 3);
	}

	BOOST_FIXTURE_TEST_CASE(test_load_from_setting_file_partly, fixture) {
		// set up
		const json::value test_data = {
			{
				"common", {
					{"time_out_seconds", 100},
				}
			},
			{
				"log", {
					{"enable_file_log", false},
				}
			},
			{
				"connection_test", {
					{"connection_check_udp_try_count", 30}
				}
			}
		};
		create_setting_file(test_data);

		// exercise
		server_setting setting;
		setting.load_from_setting_file(setting_path);

		// verify
		BOOST_CHECK_EQUAL(setting.common.enable_session_key_check, true);
		BOOST_CHECK_EQUAL(setting.common.time_out_seconds, 100);
		BOOST_CHECK(setting.common.ip_version == ip_version::v4);
		BOOST_CHECK_EQUAL(setting.common.port, 57000);
		BOOST_CHECK_EQUAL(setting.common.max_connection_per_thread, 1000);
		BOOST_CHECK_EQUAL(setting.common.thread, 1);
		BOOST_CHECK_EQUAL(setting.common.max_room_count, 1000);
		BOOST_CHECK_EQUAL(setting.common.max_player_per_room, 16);
		BOOST_CHECK_EQUAL(setting.log.enable_console_log, true);
		BOOST_CHECK(setting.log.console_log_level == log_level::info);
		BOOST_CHECK_EQUAL(setting.log.enable_file_log, false);
		BOOST_CHECK(setting.log.file_log_level == log_level::info);
		BOOST_CHECK_EQUAL(setting.log.file_log_path, "");
		BOOST_CHECK_EQUAL(setting.connection_test.connection_check_tcp_time_out_seconds, 5);
		BOOST_CHECK_EQUAL(setting.connection_test.connection_check_udp_time_out_seconds, 3);
		BOOST_CHECK_EQUAL(setting.connection_test.connection_check_udp_try_count, 30);
	}

	BOOST_DATA_TEST_CASE_F(fixture, test_load_from_setting_time_out_seconds_valid, unit_test::data::make({ 1,3600 })) {
		// set up
		const json::value test_data = {
			{
				"common", {
					{"time_out_seconds", sample},
				}
			}
		};
		create_setting_file(test_data);

		// exercise
		server_setting setting;
		setting.load_from_setting_file(setting_path);

		// verify
		BOOST_CHECK_EQUAL(setting.common.time_out_seconds, sample);
	}

	BOOST_DATA_TEST_CASE_F(fixture, test_load_from_setting_ip_version_valid, unit_test::data::make({ "v4", "v6"})) {
		// set up
		const json::value test_data = {
			{
				"common", {
					{"ip_version", sample},
				}
			}
		};
		create_setting_file(test_data);

		// exercise
		server_setting setting;
		setting.load_from_setting_file(setting_path);

		// verify
		BOOST_CHECK(setting.common.ip_version == string_to_ip_version(sample));
	}

	BOOST_DATA_TEST_CASE_F(fixture, test_load_from_setting_port_valid, unit_test::data::make({ 0,65535 })) {
		// set up
		const json::value test_data = {
			{
				"common", {
					{"port", sample},
				}
			}
		};
		create_setting_file(test_data);

		// exercise
		server_setting setting;
		setting.load_from_setting_file(setting_path);

		// verify
		BOOST_CHECK_EQUAL(setting.common.port, sample);
	}

	BOOST_DATA_TEST_CASE_F(fixture, test_load_from_setting_max_connection_per_thread_valid,
		unit_test::data::make({ 1,65535 })) {
		// set up
		const json::value test_data = {
			{
				"common", {
					{"max_connection_per_thread", sample},
				}
			}
		};
		create_setting_file(test_data);

		// exercise
		server_setting setting;
		setting.load_from_setting_file(setting_path);

		// verify
		BOOST_CHECK_EQUAL(setting.common.max_connection_per_thread, sample);
	}

	BOOST_DATA_TEST_CASE_F(fixture, test_load_from_setting_thread_valid, unit_test::data::make({ 1,65535 })) {
		// set up
		const json::value test_data = {
			{
				"common", {
					{"thread", sample},
				}
			}
		};
		create_setting_file(test_data);

		// exercise
		server_setting setting;
		setting.load_from_setting_file(setting_path);

		// verify
		BOOST_CHECK_EQUAL(setting.common.thread, sample);
	}

	BOOST_DATA_TEST_CASE_F(fixture, test_load_from_setting_max_room_count_valid, unit_test::data::make({ 1,65535 })) {
		// set up
		const json::value test_data = {
			{
				"common", {
					{"max_room_count", sample},
				}
			}
		};
		create_setting_file(test_data);

		// exercise
		server_setting setting;
		setting.load_from_setting_file(setting_path);

		// verify
		BOOST_CHECK_EQUAL(setting.common.max_room_count, sample);
	}

	BOOST_DATA_TEST_CASE_F(fixture, test_load_from_setting_max_player_per_room_valid,
		unit_test::data::make({ 1,255 })) {
		// set up
		const json::value test_data = {
			{
				"common", {
					{"max_player_per_room", sample},
				}
			}
		};
		create_setting_file(test_data);

		// exercise
		server_setting setting;
		setting.load_from_setting_file(setting_path);

		// verify
		BOOST_CHECK_EQUAL(setting.common.max_player_per_room, sample);
	}

	BOOST_DATA_TEST_CASE_F(fixture, test_load_from_setting_console_log_level_valid,
		unit_test::data::make({ "debug", "info", "warning", "error", "fatal"})) {
		// set up
		const json::value test_data = {
			{
				"log", {
					{"console_log_level", sample},
				}
			}
		};
		create_setting_file(test_data);

		// exercise
		server_setting setting;
		setting.load_from_setting_file(setting_path);

		// verify
		BOOST_CHECK(setting.log.console_log_level== string_to_log_level(sample));
	}

	BOOST_DATA_TEST_CASE_F(fixture, test_load_from_setting_file_log_level_valid,
		unit_test::data::make({ "debug", "info", "warning", "error", "fatal" })) {
		// set up
		const json::value test_data = {
			{
				"log", {
					{"file_log_level", sample},
				}
			}
		};
		create_setting_file(test_data);

		// exercise
		server_setting setting;
		setting.load_from_setting_file(setting_path);

		// verify
		BOOST_CHECK(setting.log.file_log_level == string_to_log_level(sample));
	}

	BOOST_DATA_TEST_CASE_F(fixture, test_load_from_setting_connection_check_tcp_time_out_seconds_valid,
		unit_test::data::make({ 1,3600})) {
		// set up
		const json::value test_data = {
			{
				"connection_test", {
					{"connection_check_tcp_time_out_seconds", sample},
				}
			}
		};
		create_setting_file(test_data);

		// exercise
		server_setting setting;
		setting.load_from_setting_file(setting_path);

		// verify
		BOOST_CHECK_EQUAL(setting.connection_test.connection_check_tcp_time_out_seconds, sample);
	}

	BOOST_DATA_TEST_CASE_F(fixture, test_load_from_setting_connection_check_udp_time_out_seconds_valid,
		unit_test::data::make({ 1,3600 })) {
		// set up
		const json::value test_data = {
			{
				"connection_test", {
					{"connection_check_udp_time_out_seconds", sample},
				}
			}
		};
		create_setting_file(test_data);

		// exercise
		server_setting setting;
		setting.load_from_setting_file(setting_path);

		// verify
		BOOST_CHECK_EQUAL(setting.connection_test.connection_check_udp_time_out_seconds, sample);
	}

	BOOST_DATA_TEST_CASE_F(fixture, test_load_from_setting_connection_check_udp_try_count_valid,
		unit_test::data::make({ 1,100 })) {
		// set up
		const json::value test_data = {
			{
				"connection_test", {
					{"connection_check_udp_try_count", sample},
				}
			}
		};
		create_setting_file(test_data);

		// exercise
		server_setting setting;
		setting.load_from_setting_file(setting_path);

		// verify
		BOOST_CHECK_EQUAL(setting.connection_test.connection_check_udp_try_count, sample);
	}

	BOOST_DATA_TEST_CASE_F(fixture, test_load_from_setting_invalid_integer_value,
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
		const json::value test_data = {
			{
				section, {
					{key, value},
				}
			}
		};
		create_setting_file(test_data);

		// exercise and verify
		server_setting setting;
		BOOST_CHECK_THROW(setting.load_from_setting_file(setting_path), server_setting_error);
	}

	BOOST_DATA_TEST_CASE_F(fixture, test_load_from_setting_invalid_string_value,
		unit_test::data::make({
			std::tuple{"common", "ip_version", "v5"},
			std::tuple{"common", "ip_version", "4"},
			std::tuple{"common", "ip_version", "6"},
			std::tuple{"log", "console_log_level", "inf"},
			std::tuple{"log", "file_log_level", "inf"},
			}), section, key, value) {
		// set up
		const json::value test_data = {
			{
				section, {
					{key, value},
				}
			}
		};
		create_setting_file(test_data);

		// exercise and verify
		server_setting setting;
		BOOST_CHECK_THROW(setting.load_from_setting_file(setting_path), server_setting_error);
	}

BOOST_AUTO_TEST_SUITE_END()