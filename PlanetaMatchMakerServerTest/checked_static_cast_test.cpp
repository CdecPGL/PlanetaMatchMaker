#include <boost/test/unit_test.hpp>

#include "../PlanetaMatchMakerServer/source/utilities/checked_static_cast.hpp"

BOOST_AUTO_TEST_SUITE(checked_static_cast_test)
	BOOST_AUTO_TEST_CASE(test_range_checked_static_cast_no_error) {
		constexpr uint8_t original{11};
		BOOST_CHECK_EQUAL(original, pgl::range_checked_static_cast<uint8_t>(original));
	}

	BOOST_AUTO_TEST_CASE(test_range_checked_static_cast_no_error_max_value) {
		constexpr uint8_t original{255};
		BOOST_CHECK_EQUAL(original, pgl::range_checked_static_cast<uint8_t>(original));
	}

	BOOST_AUTO_TEST_CASE(test_range_checked_static_cast_no_error_min_value) {
		constexpr uint8_t original{0};
		BOOST_CHECK_EQUAL(original, pgl::range_checked_static_cast<uint8_t>(original));
	}

	BOOST_AUTO_TEST_CASE(test_range_checked_static_cast_no_error_of_larger_cast) {
		constexpr uint8_t original{0};
		BOOST_CHECK_EQUAL(original, pgl::range_checked_static_cast<uint32_t>(original));
	}

	BOOST_AUTO_TEST_CASE(test_range_checked_static_cast_no_error_of_smaller_cast) {
		constexpr uint32_t original{255};
		BOOST_CHECK_EQUAL(original, pgl::range_checked_static_cast<uint8_t>(original));
	}

	BOOST_AUTO_TEST_CASE(test_range_checked_static_cast_no_error_of_sign_cast) {
		constexpr int8_t original{127};
		BOOST_CHECK_EQUAL(original, pgl::range_checked_static_cast<uint8_t>(original));
	}

	BOOST_AUTO_TEST_CASE(test_range_checked_static_cast_error_of_overflow) {
		constexpr uint32_t original{256};
		BOOST_CHECK_EXCEPTION(pgl::range_checked_static_cast<uint8_t>(original), pgl::static_cast_range_error,
			[](auto) {return true; });
	}

	BOOST_AUTO_TEST_CASE(test_range_checked_static_cast_error_of_sigen_change) {
		constexpr int8_t original{-1};
		BOOST_CHECK_EXCEPTION(pgl::range_checked_static_cast<uint8_t>(original), pgl::static_cast_range_error,
			[](auto) {return true; });
	}

BOOST_AUTO_TEST_SUITE_END()
