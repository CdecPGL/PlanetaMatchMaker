#include <boost/test/unit_test.hpp>

#include <stdexcept>

#include "../PlanetaMatchMakerServer/source/session/session_data.hpp"

BOOST_AUTO_TEST_SUITE(session_data_test)
	BOOST_AUTO_TEST_CASE(test_session_number_is_empty_by_default) {
		const pgl::session_data session_data;

		BOOST_CHECK(!session_data.session_number().has_value());
	}

	BOOST_AUTO_TEST_CASE(test_set_session_number_stores_value) {
		pgl::session_data session_data;

		session_data.set_session_number(42);

		BOOST_REQUIRE(session_data.session_number().has_value());
		BOOST_CHECK_EQUAL(*session_data.session_number(), 42);
	}

	BOOST_AUTO_TEST_CASE(test_set_session_number_rejects_reassignment) {
		pgl::session_data session_data;
		session_data.set_session_number(42);

		BOOST_CHECK_EXCEPTION(session_data.set_session_number(43), std::runtime_error,
			[](const auto&) { return true; });
	}

BOOST_AUTO_TEST_SUITE_END()
