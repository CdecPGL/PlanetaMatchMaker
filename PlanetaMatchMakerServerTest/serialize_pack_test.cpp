#include <boost/test/unit_test.hpp>

#include "../PlanetaMatchMakerServer/source/utilities/pack.hpp"

BOOST_AUTO_TEST_SUITE(serialize_pack_test)

	struct test_struct1 final {
		uint8_t value1;
		uint64_t value2;
		std::array<uint8_t, 4> value3;

		using serialize_targets = minimal_serializer::serialize_target_container<
			&test_struct1::value1,
			&test_struct1::value2,
			&test_struct1::value3
		>;
	};

	struct test_struct2 final {
		int64_t value1;
		std::array<uint16_t, 8> value2;

		using serialize_targets = minimal_serializer::serialize_target_container<
			&test_struct2::value1,
			&test_struct2::value2
		>;
	};

	BOOST_AUTO_TEST_CASE(test_packed_one_data_size) {
		using test_t = test_struct1;
		const test_t data{};
		const auto expected = minimal_serializer::serialized_size_v<test_t>;
		const auto actual = pgl::pack_data(data).size();
		BOOST_CHECK_EQUAL(expected, actual);
	}

	BOOST_AUTO_TEST_CASE(test_packed_two_data_size) {
		using test_t1 = test_struct1;
		using test_t2 = test_struct2;
		const test_t1 data1{};
		const test_t2 data2{};
		const auto expected = minimal_serializer::serialized_size_v<test_t1> + minimal_serializer::serialized_size_v<
			test_t2>;
		const auto actual = pgl::pack_data(data1, data2).size();
		BOOST_CHECK_EQUAL(expected, actual);
	}

	BOOST_AUTO_TEST_CASE(test_pack_and_unpack_one_data) {
		using test_t = uint64_t;
		const test_t expected{123};

		const auto actual_buffer = pgl::pack_data(expected);

		test_t actual;
		pgl::unpack_data(actual_buffer, actual);

		BOOST_CHECK_EQUAL(expected, actual);
	}

	BOOST_AUTO_TEST_CASE(test_pack_and_unpack_two_data) {
		using test_t1 = uint64_t;
		using test_t2 = int64_t;
		const test_t1 expected1{123};
		const test_t2 expected2{-23452123};

		const auto actual_buffer = pgl::pack_data(expected1, expected2);

		test_t1 actual1;
		test_t2 actual2;
		pgl::unpack_data(actual_buffer, actual1, actual2);

		BOOST_CHECK_EQUAL(expected1, actual1);
		BOOST_CHECK_EQUAL(expected2, actual2);
	}

	BOOST_AUTO_TEST_CASE(test_unpack_lack_of_data_exception) {
		using test_t = uint64_t;
		test_t actual;
		BOOST_CHECK_EXCEPTION(pgl::unpack_data(std::vector<uint8_t>(1), actual),
			minimal_serializer::serialization_error,
			[](auto) {return true; });
	}

BOOST_AUTO_TEST_SUITE_END()
