#include <boost/test/unit_test.hpp>

#include "../../PlanetaMatchMakerServer/source/server/server_data.hpp"

BOOST_AUTO_TEST_SUITE(server_data_test)
	BOOST_AUTO_TEST_CASE(test_issue_session_number_returns_sequential_values) {
		pgl::server_data server_data;

		BOOST_CHECK_EQUAL(server_data.issue_session_number(), 1);
		BOOST_CHECK_EQUAL(server_data.issue_session_number(), 2);
		BOOST_CHECK_EQUAL(server_data.issue_session_number(), 3);
	}

	BOOST_AUTO_TEST_CASE(test_get_room_data_container_returns_shared_storage) {
		pgl::server_data server_data;
		const auto& const_server_data = server_data;
		pgl::room_data room_data{
			1,
			{u8"host", 1},
			pgl::room_setting_flag::public_room | pgl::room_setting_flag::open_room,
			{},
			4,
			pgl::datetime(2024, 1, 1),
			{},
			pgl::game_host_connection_establish_mode::builtin,
			{},
			{},
			1
		};

		server_data.get_room_data_container().add_or_update(room_data);

		BOOST_CHECK(const_server_data.get_room_data_container().contains(1));
	}

	BOOST_AUTO_TEST_CASE(test_get_player_name_container_returns_shared_storage) {
		pgl::server_data server_data;
		const auto& const_server_data = server_data;

		const auto player_full_name = server_data.get_player_name_container().assign_player_name(u8"player");

		BOOST_CHECK(const_server_data.get_player_name_container().is_player_exist(player_full_name));
	}

BOOST_AUTO_TEST_SUITE_END()
