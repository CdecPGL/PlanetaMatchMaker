#include <boost/test/unit_test.hpp>

#include "../PlanetaMatchMakerServer/source/utilities/string_utility.hpp"

BOOST_AUTO_TEST_SUITE(string_utilities_test)
	BOOST_AUTO_TEST_CASE(test_string_concatenation) {
		BOOST_CHECK_EQUAL(u8"ABCXYZ������", pgl::generate_string(u8"ABC", u8"XYZ", u8"������"));
	}

	BOOST_AUTO_TEST_CASE(test_integer_concatenation) {
		BOOST_CHECK_EQUAL(u8"123-456", pgl::generate_string(123, -456));
	}

	BOOST_AUTO_TEST_CASE(test_float_concatenation) {
		BOOST_CHECK_EQUAL(u8"0.12-0.34", pgl::generate_string(0.12f, -0.34f));
	}

	BOOST_AUTO_TEST_CASE(test_double_concatenation) {
		// double is rounded by 6 digits
		BOOST_CHECK_EQUAL(u8"0.123457-0.987654", pgl::generate_string(0.123456789, -0.987654321));
	}

	BOOST_AUTO_TEST_CASE(test_bool_concatenation) {
		BOOST_CHECK_EQUAL(u8"truefalse", pgl::generate_string(true, false));
	}

	enum class test_enum { element1, element2 };

	BOOST_AUTO_TEST_CASE(test_enum_concatenation) {
		BOOST_CHECK_EQUAL(u8"element1element2", pgl::generate_string(test_enum::element1, test_enum::element2));
	}

	BOOST_AUTO_TEST_CASE(test_mixed_concatenation) {
		// double is rounded by 6 digits
		BOOST_CHECK_EQUAL(u8"test1true0.230.888889element1",
			pgl::generate_string("test", 1, true, 0.23, 0.8888888888, test_enum::element1));
	}

	BOOST_AUTO_TEST_CASE(test_empty_parameter) {
		BOOST_CHECK_EQUAL(u8"", pgl::generate_string());
	}

	BOOST_AUTO_TEST_CASE(test_one_parameter) {
		BOOST_CHECK_EQUAL(u8"1", pgl::generate_string(1));
	}

	BOOST_AUTO_TEST_CASE(test_many_parameters) {
		BOOST_CHECK_EQUAL(u8"12345678910", pgl::generate_string(1, 2, 3, 4, 5, 6, 7, 8, 9, 10));
	}

BOOST_AUTO_TEST_SUITE_END()