#include <limits>

#include <boost/test/unit_test.hpp>

#include "../PlanetaMatchMakerServer/source/serialize/serializer.hpp"
#include <boost/mpl/list.hpp>

// 22 bytes
struct simple_struct_member_serialize final {
	const static size_t size = 22;

	std::array<int32_t, 5> value1;
	uint16_t value2;

	void on_serialize(pgl::serializer& serializer) {
		serializer += value1;
		serializer += value2;
	}

	bool operator==(const simple_struct_member_serialize& other) const {
		return value1 == other.value1 &&
			value2 == other.value2;
	}

	static simple_struct_member_serialize get_default() {
		return {
			{10, 11, 12, 13, 14},
			8777
		};
	}
};

// 22 bytes
struct simple_struct_global_serialize final {
	const static size_t size = 22;

	std::array<int32_t, 5> value1;
	uint16_t value2;

	bool operator==(const simple_struct_global_serialize& other) const {
		return value1 == other.value1 &&
			value2 == other.value2;
	}

	static simple_struct_global_serialize get_default() {
		return {
			{10, 11, 12, 13, 14},
			8777
		};
	}
};

namespace pgl {
	void on_serialize(simple_struct_global_serialize& data, pgl::serializer& serializer) {
		serializer += data.value1;
		serializer += data.value2;
	}
}

// 94 bytes
struct nested_struct final {
	const static size_t size = 94;

	uint8_t value1;
	int64_t value2;
	bool value3;
	std::array<uint32_t, 10> value4;
	simple_struct_member_serialize value5;
	simple_struct_global_serialize value6;

	void on_serialize(pgl::serializer& serializer) {
		serializer += value1;
		serializer += value2;
		serializer += value3;
		serializer += value4;
		serializer += value5;
		serializer += value6;
	}

	bool operator==(const nested_struct& other) const {
		return value1 == other.value1 &&
			value2 == other.value2 &&
			value3 == other.value3 &&
			value4 == other.value4 &&
			value5 == other.value5 &&
			value6 == other.value6;
	}

	static nested_struct get_default() {
		return {
			255,
			-345345346,
			true,
			{1, 2, 3, 4, 5, 6, 7, 8, 9, 10},
			{{10, 11, 12, 13, 14}, 8777},
			{{10, 11, 12, 13, 14}, 8777}
		};
	}
};

enum test_enum { a, b, c };

enum class test_enum_class { x, y, z };

template <typename T>
auto get_default() -> std::enable_if_t<std::is_arithmetic_v<T>, T> {
	if constexpr (std::is_same_v<T, bool>) {
		return true;
	} else {
		if constexpr (std::is_signed_v<T>) {
			return std::numeric_limits<T>::min() / 10;
		} else {
			return std::numeric_limits<T>::max() / 10;
		}
	}
}

template <typename T>
auto get_default() -> std::enable_if_t<std::is_enum_v<T>, T> {
	return static_cast<T>(1);
}

template <typename T>
auto get_default() -> decltype(T::get_default, T{}) {
	return T::get_default();
}

template <typename T>
auto get_size() -> std::enable_if_t<std::is_arithmetic_v<T> || std::is_enum_v<T>, size_t> {
	return sizeof(T);
}

template <typename T>
auto get_size() -> decltype(T::size, size_t{}) {
	return T::size;
}

BOOST_AUTO_TEST_SUITE(serialize_test)
	typedef boost::mpl::list<int8_t, uint8_t, int16_t, uint16_t,
		int32_t, uint32_t, int64_t, uint64_t, bool,
		test_enum, test_enum_class,
		simple_struct_member_serialize, simple_struct_global_serialize, nested_struct>
	test_types;

	BOOST_AUTO_TEST_CASE_TEMPLATE(test_member_serialize_not_change, Test, test_types) {
		const auto expected = get_default<Test>();
		const auto actual = expected;
		pgl::serialize(actual);
		BOOST_CHECK(expected == actual);
	}

	BOOST_AUTO_TEST_CASE_TEMPLATE(test_member_serialize_consistency, Test, test_types) {
		const auto expected = get_default<Test>();
		auto data1 = pgl::serialize(expected);
		auto data2 = pgl::serialize(expected);
		BOOST_CHECK_EQUAL_COLLECTIONS(data1.begin(), data1.end(), data2.begin(), data2.end());
	}

	BOOST_AUTO_TEST_CASE_TEMPLATE(test_member_serialize_deserialize, Test, test_types) {
		const auto expected = get_default<Test>();
		Test actual{};
		auto data = pgl::serialize(expected);
		pgl::deserialize(actual, data);
		BOOST_CHECK(expected == actual);
	}

	BOOST_AUTO_TEST_CASE_TEMPLATE(test_member_serialize_size, Test, test_types) {
		auto size = pgl::get_serialized_size<Test>();
		BOOST_CHECK_EQUAL(get_size<Test>(), size);
	}

	BOOST_AUTO_TEST_CASE(test_member_serialize_not_change_array) {
		const std::array<int32_t, 4> expected = {-123, 23, 56, 7};
		const auto actual = expected;
		pgl::serialize(actual);
		BOOST_CHECK(expected == actual);
	}

	BOOST_AUTO_TEST_CASE(test_member_serialize_consistency_array) {
		const std::array<int32_t, 4> expected = {-123, 23, 56, 7};
		auto data1 = pgl::serialize(expected);
		auto data2 = pgl::serialize(expected);
		BOOST_CHECK_EQUAL_COLLECTIONS(data1.begin(), data1.end(), data2.begin(), data2.end());
	}

	BOOST_AUTO_TEST_CASE(test_member_serialize_deserialize_array) {
		const std::array<int32_t, 4> expected = {-123, 23, 56, 7};
		std::array<int32_t, 4> actual{};
		auto data = pgl::serialize(expected);
		pgl::deserialize(actual, data);
		BOOST_CHECK(expected == actual);
	}

	BOOST_AUTO_TEST_CASE(test_member_serialize_size_array) {
		const std::array<int32_t, 4> expected = {-123, 23, 56, 7};
		auto size = pgl::get_serialized_size<std::array<int32_t, 4>>();
		BOOST_CHECK_EQUAL(sizeof(int32_t)*4, size);
	}

BOOST_AUTO_TEST_SUITE_END()
