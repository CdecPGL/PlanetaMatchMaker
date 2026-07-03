#include <boost/test/unit_test.hpp>

#include "../PlanetaMatchMakerServer/source/server/server_data.hpp"

BOOST_AUTO_TEST_SUITE(server_data_test)
	BOOST_AUTO_TEST_CASE(test_issue_session_number_returns_sequential_values) {
		pgl::server_data server_data;

		BOOST_CHECK_EQUAL(server_data.issue_session_number(), 1);
		BOOST_CHECK_EQUAL(server_data.issue_session_number(), 2);
		BOOST_CHECK_EQUAL(server_data.issue_session_number(), 3);
	}

BOOST_AUTO_TEST_SUITE_END()
