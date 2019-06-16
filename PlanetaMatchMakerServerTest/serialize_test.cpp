#include <boost/test/unit_test.hpp>

#include "../PlanetaMatchMakerServer/source/serialize/serializer.hpp"

BOOST_AUTO_TEST_SUITE(serialize_test)

	struct test_struct1 final {
		std::array<int, 5> value1;
		uint16_t value2;

		void on_serialize(pgl::serializer& serializer) {
			serializer += value1;
			serializer += value2;
		}

		bool operator==(const test_struct1& other) const {
			return value1 == other.value1 &&
				value2 == other.value2;
		}
	};

	struct test_struct2 final {
		uint8_t value1;
		int64_t value2;
		bool value3;
		std::array<uint32_t, 10> value4;
		test_struct1 value5;

		void on_serialize(pgl::serializer& serializer) {
			serializer += value1;
			serializer += value2;
			serializer += value3;
			serializer += value4;
			serializer += value5;
		}

		bool operator==(const test_struct2& other) const {
			return value1 == other.value1 &&
				value2 == other.value2 &&
				value3 == other.value3 &&
				value4 == other.value4 &&
				value5 == other.value5;
		}
	};

	BOOST_AUTO_TEST_CASE(test_serialize_not_change) {
		const test_struct2 expected{
			255,
			-345345346,
			true,
			{1, 2, 3, 4, 5, 6, 7, 8, 9, 10},
			{{10, 11, 12, 13, 14}, 8777}
		};

		const auto actual = expected;
		pgl::serialize(actual);
		BOOST_CHECK(expected == actual);
	}

	BOOST_AUTO_TEST_CASE(test_serialization) {
		const test_struct2 expected{
			255,
			-345345346,
			true,
			{1, 2, 3, 4, 5, 6, 7, 8, 9, 10},
			{{10, 11, 12, 13, 14}, 8777}
		};

		test_struct2 actual{};
		auto data = pgl::serialize(expected);
		pgl::deserialize(actual, data);
		BOOST_CHECK(expected == actual);
	}

BOOST_AUTO_TEST_SUITE_END()
