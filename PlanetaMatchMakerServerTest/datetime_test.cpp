#include <boost/test/unit_test.hpp>

#include "../PlanetaMatchMakerServer/source/datetime/datetime.hpp"

BOOST_AUTO_TEST_SUITE(datetime_test)
	BOOST_AUTO_TEST_CASE(test_year_getter) {
		const pgl::datetime datetime(2019, 5, 6, 11, 22, 33);
		BOOST_CHECK_EQUAL(2019, datetime.year());
	}

	BOOST_AUTO_TEST_CASE(test_month_getter) {
		const pgl::datetime datetime(2019, 5, 6, 11, 22, 33);
		BOOST_CHECK_EQUAL(5, datetime.month());
	}

	BOOST_AUTO_TEST_CASE(test_day_getter) {
		const pgl::datetime datetime(2019, 5, 6, 11, 22, 33);
		BOOST_CHECK_EQUAL(6, datetime.day());
	}

	BOOST_AUTO_TEST_CASE(test_hour_getter) {
		const pgl::datetime datetime(2019, 5, 6, 11, 22, 33);
		BOOST_CHECK_EQUAL(11, datetime.hour());
	}

	BOOST_AUTO_TEST_CASE(test_minute_getter) {
		const pgl::datetime datetime(2019, 5, 6, 11, 22, 33);
		BOOST_CHECK_EQUAL(22, datetime.minute());
	}

	BOOST_AUTO_TEST_CASE(test_second_getter) {
		const pgl::datetime datetime(2019, 5, 6, 11, 22, 33);
		BOOST_CHECK_EQUAL(33, datetime.second());
	}

	BOOST_AUTO_TEST_CASE(test_less_than_operator_true) {
		const pgl::datetime small(2019, 5, 6, 11, 22, 33);
		const pgl::datetime big(2019, 5, 6, 11, 23, 33);
		BOOST_CHECK(small < big);
	}

	BOOST_AUTO_TEST_CASE(test_less_than_operator_false) {
		const pgl::datetime small(2019, 5, 6, 11, 22, 33);
		const pgl::datetime big(2019, 5, 6, 11, 23, 33);
		BOOST_CHECK(!(big < small));
	}

	BOOST_AUTO_TEST_CASE(test_less_than_operator_same) {
		const pgl::datetime test1(2019, 5, 6, 11, 22, 33);
		const pgl::datetime test2(2019, 5, 6, 11, 22, 33);
		BOOST_CHECK(!(test1 < test2));
	}

	BOOST_AUTO_TEST_CASE(test_equal_operator_true) {
		const pgl::datetime test1(2019, 5, 6, 11, 22, 33);
		const pgl::datetime test2(2019, 5, 6, 11, 22, 33);
		BOOST_CHECK(test1 == test2);
	}

	BOOST_AUTO_TEST_CASE(test_equal_operator_false) {
		const pgl::datetime test1(2019, 5, 6, 11, 22, 33);
		const pgl::datetime test2(2019, 5, 6, 11, 23, 33);
		BOOST_CHECK(!(test1 == test2));
	}

BOOST_AUTO_TEST_SUITE_END()
