#include <boost/test/unit_test.hpp>

#include "../PlanetaMatchMakerServer/source/async/read_write.hpp"

BOOST_AUTO_TEST_SUITE(async_read_write_test)

	struct test_struct1 final {
		uint8_t value1;
		uint64_t value2;
		std::array<uint8_t, 4> value3;
	};

	struct test_struct2 final {
		int64_t value1;
		std::array<uint16_t, 8> value2;
	};

	BOOST_AUTO_TEST_CASE(test_packed_two_data_size) {
		using test_t1 = test_struct1;
		using test_t2 = test_struct2;
		const auto expected = sizeof(test_t1) + sizeof(test_t2);
		const auto actual = sizeof(pgl::packed_data_buffer_t<test_t1, test_t2>);
		BOOST_CHECK_EQUAL(expected, actual);
	}

	BOOST_AUTO_TEST_CASE(test_packed_one_data_size) {
		using test_t = test_struct1;
		const auto expected = sizeof(test_t);
		const auto actual = sizeof(pgl::packed_data_buffer_t<test_t>);
		BOOST_CHECK_EQUAL(expected, actual);
	}

	BOOST_AUTO_TEST_CASE(test_pack_one_data) {
		using test_t = uint64_t;
		const test_t expected{123};

		pgl::packed_data_buffer_t<test_t> actual_buffer;
		pgl::pack_data(actual_buffer, expected);

		const auto actual = *reinterpret_cast<test_t*>(actual_buffer.data());
		BOOST_CHECK_EQUAL(expected, actual);
	}

	struct test_t1_t2 {
		using test_t1 = uint64_t;
		using test_t2 = int64_t;
		test_t1 value1;
		test_t2 value2;
	};

	BOOST_AUTO_TEST_CASE(test_pack_two_data) {
		const test_t1_t2::test_t1 expected1{123};
		const test_t1_t2::test_t2 expected2{-23452123};

		pgl::packed_data_buffer_t<test_t1_t2::test_t1, test_t1_t2::test_t2> actual_buffer;
		pgl::pack_data(actual_buffer, expected1, expected2);

		const auto actual_pack = *reinterpret_cast<test_t1_t2*>(actual_buffer.data());
		const auto actual1 = actual_pack.value1;
		const auto actual2 = actual_pack.value2;

		BOOST_CHECK_EQUAL(expected1, actual1);
		BOOST_CHECK_EQUAL(expected2, actual2);
	}

BOOST_AUTO_TEST_SUITE_END()
