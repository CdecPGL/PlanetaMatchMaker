#include <array>

#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>
#include <minimal_serializer/serializer.hpp>
#include <minimal_serializer/fixed_string.hpp>

#include "../PlanetaMatchMakerServer/source/data/thread_safe_data_container.hpp"

using namespace boost;
using namespace pgl;

struct test_struct1 final {
	uint8_t id;
	float value1;
	bool value2;
	std::array<uint16_t, 4> value3;
	minimal_serializer::fixed_u8string<24> unique1;
	int32_t unique2;

	bool operator==(const test_struct1& other) const {
		return id == other.id && value1 == other.value1 && value2 == other.value2 && value3 == other.value3 && unique1
			== other.unique1 && unique2 == other.unique2;
	}
};

template <typename T, size_t S>
std::ostream& operator<<(std::ostream& os, const std::array<T, S>& d) {
	os << "[";
	for (auto&& v : d) { os << v << ","; }
	os << "]";
	return os;
}

std::ostream& operator<<(std::ostream& os, const test_struct1& d) {
	os << nameof::nameof_type<test_struct1>() << "{" << d.id << "," << d.value1 << "," << d.value2 << "," << d.value3 <<
		", " << d.unique1 << "," << d.unique2 << "}";
	return os;
}

template <typename T>
std::ostream& operator<<(std::ostream& os, const std::vector<T>& d) {
	os << "[";
	for (auto&& v : d) { os << v << ","; }
	os << "]";
	return os;
}

using container_t = thread_safe_data_container<test_struct1, &test_struct1::id, &test_struct1::unique1, &
	test_struct1::unique2>;

const auto data1 = test_struct1{
	123, 1.2f, true, {6, 7, 8}, u8"test1", 1
};
const auto data2 = test_struct1{
	3, 1.2f, true, {6, 7, 8}, u8"test2", 2
};
const auto data3 = test_struct1{
	4, -1.2f, true, {6, 7, 8}, u8"test3", 3
};
const auto data4 = test_struct1{
	100, 1.2f, true, {6, 7, 8}, u8"test4", 4
};

BOOST_AUTO_TEST_SUITE(thread_safe_data_container_test)
	////////////////////////////////
	// add_data, get_data 
	////////////////////////////////

	BOOST_AUTO_TEST_CASE(test_add_and_get) {
		// set up
		auto container = container_t();

		// exercise
		container.add_data(data1.id, data1);
		container.add_data(data2.id, data2);
		const auto actual1 = container.get_data(data1.id);
		const auto actual2 = container.get_data(data2.id);

		// verify
		BOOST_CHECK_EQUAL(actual1, data1);
		BOOST_CHECK_EQUAL(actual2, data2);
	}

	////////////////////////////////
	// add_data 
	////////////////////////////////

	BOOST_AUTO_TEST_CASE(test_add_already_exist_id) {
		// set up
		auto container = container_t();
		auto duplicate_data = data2;
		duplicate_data.id = data1.id;
		container.add_data(data1.id, data1);

		// exercise and verify
		BOOST_CHECK_THROW(container.add_data(duplicate_data.id, duplicate_data), unique_variable_duplication_error);
	}

	BOOST_DATA_TEST_CASE(test_add_already_exist_idunique_variable, unit_test::data::make({
		test_struct1{19, -13.2f, false, {64, 47, 84}, u8"test1", 0},
		test_struct1{19, -13.2f, false, {64, 47, 84}, u8"test", 1},
		}), duplicate_data) {
		// set up
		auto container = container_t();
		container.add_data(data1.id, data1);

		// exercise and verify
		BOOST_CHECK_THROW(container.add_data(duplicate_data.id, duplicate_data), unique_variable_duplication_error);
	}

	////////////////////////////////
	// assign_id_and_add_data, get_data 
	////////////////////////////////

	BOOST_AUTO_TEST_CASE(test_assign_id_and_add_and_get) {
		// set up
		auto container = container_t();
		auto non_const_data1 = data1;
		auto non_const_data2 = data2;
		const auto id_setter = [](test_struct1& data, const uint8_t& id) { data.id = id; };

		// exercise
		const auto id1 = container.assign_id_and_add_data(non_const_data1, id_setter);
		const auto id2 = container.assign_id_and_add_data(non_const_data2, id_setter);
		const auto actual1 = container.get_data(non_const_data1.id);
		const auto actual2 = container.get_data(non_const_data2.id);

		// verify
		non_const_data1.id = id1;
		non_const_data2.id = id2;
		BOOST_CHECK_EQUAL(actual1, non_const_data1);
		BOOST_CHECK_EQUAL(actual2, non_const_data2);
	}

	////////////////////////////////
	// assign_id_and_add_data 
	////////////////////////////////

	BOOST_DATA_TEST_CASE(test_assign_id_and_add_already_exist_unique_variable, unit_test::data::make({
		test_struct1{19, -13.2f, false, {64, 47, 84}, u8"test1", 0},
		test_struct1{19, -13.2f, false, {64, 47, 84}, u8"test", 1},
		}), duplicate_data) {
		// set up
		auto container = container_t();
		auto non_const_data1 = data1;
		const auto id_setter = [](test_struct1& data, const uint8_t& id) { data.id = id; };
		container.assign_id_and_add_data(non_const_data1, id_setter);

		// exercise and verify
		auto non_const_duplicate_data = duplicate_data;
		BOOST_CHECK_THROW(container.assign_id_and_add_data(non_const_duplicate_data, id_setter),
			unique_variable_duplication_error);
	}

	////////////////////////////////
	// get_data 
	////////////////////////////////

	BOOST_AUTO_TEST_CASE(test_get_not_exist_data) {
		// set up
		auto container = container_t();
		container.add_data(data1.id, data1);

		// exercise and verify
		// ReSharper disable once CppNoDiscardExpression
		BOOST_CHECK_THROW(container.get_data(9), std::out_of_range); // NOLINT(clang-diagnostic-unused-result)
	}

	BOOST_AUTO_TEST_CASE(test_get_filterd) {
		// set up
		auto container = container_t();
		const auto expected = std::vector{data2, data4, data1};
		container.add_data(data1.id, data1);
		container.add_data(data2.id, data2);
		container.add_data(data3.id, data3);
		container.add_data(data4.id, data4);

		// exercise
		const auto result = container.get_data(
			[](const auto& l, const auto& r) { return l.id < r.id; },
			[](const auto& d) { return d.value1 >= 0; }
		);

		// verify
		BOOST_CHECK_EQUAL_COLLECTIONS(result.begin(), result.end(), expected.begin(), expected.end());
	}

	BOOST_AUTO_TEST_CASE(test_get_filterd_no_data) {
		// set up
		const auto container = container_t();
		const auto expected = std::vector<test_struct1>{};

		// exercise
		const auto result = container.get_data(
			[](const auto& l, const auto& r) { return l.id < r.id; },
			[](const auto& d) { return d.value1 >= 0; }
		);

		// verify
		BOOST_CHECK_EQUAL_COLLECTIONS(result.begin(), result.end(), expected.begin(), expected.end());
	}

	////////////////////////////////
	// get_range_data 
	////////////////////////////////

	BOOST_DATA_TEST_CASE(test_get_ranged, unit_test::data::make({
		std::tuple{0, 3, std::vector{ data2, data4, data1 }}, // full in range
		std::tuple{1, 1, std::vector{ data4}}, // partly in range
		std::tuple{1, 3, std::vector{ data4, data1}}, // count over
		std::tuple{3, 1, std::vector<test_struct1>{ }}, // out of range
		std::tuple{0, 0, std::vector<test_struct1>{ }}, // zero range
		}
	), start_idx, count, expected) {
		// set up
		auto container = container_t();
		container.add_data(data1.id, data1);
		container.add_data(data2.id, data2);
		container.add_data(data3.id, data3);
		container.add_data(data4.id, data4);

		// exercise
		const auto result = container.get_range_data(
			start_idx, count,
			[](const auto& l, const auto& r) { return l.id < r.id; },
			[](const auto& d) { return d.value1 >= 0; }
		);

		// verify
		BOOST_CHECK_EQUAL_COLLECTIONS(result.begin(), result.end(), expected.begin(), expected.end());
	}

	////////////////////////////////
	// update_data 
	////////////////////////////////

	BOOST_AUTO_TEST_CASE(test_update) {
		// set up
		auto container = container_t();
		auto updated_data1 = data3;
		updated_data1.id = data1.id;
		auto updated_data2 = data4;
		updated_data2.id = data2.id;
		container.add_data(data1.id, data1);
		container.add_data(data2.id, data2);

		// exercise
		container.update_data(updated_data1.id, updated_data1);
		container.update_data(updated_data2.id, updated_data2);
		const auto actual1 = container.get_data(updated_data1.id);
		const auto actual2 = container.get_data(updated_data2.id);

		// verify
		BOOST_CHECK_EQUAL(actual1, updated_data1);
		BOOST_CHECK_EQUAL(actual2, updated_data2);
	}

	BOOST_AUTO_TEST_CASE(test_update_by_same_data) {
		// set up
		auto container = container_t();
		container.add_data(data1.id, data1);

		// exercise
		container.update_data(data1.id, data1);
		const auto actual1 = container.get_data(data1.id);

		// verify
		BOOST_CHECK_EQUAL(actual1, data1);
	}

	BOOST_AUTO_TEST_CASE(test_update_not_exist_id) {
		// set up
		auto container = container_t();

		// exercise and verify
		BOOST_CHECK_THROW(container.update_data(data1.id, data1), std::out_of_range);
	}

	BOOST_DATA_TEST_CASE(test_update_already_exist_unique_variable, unit_test::data::make({
		test_struct1{19, -13.2f, false, {64, 47, 84}, u8"test1", 0},
		test_struct1{19, -13.2f, false, {64, 47, 84}, u8"test", 1},
		}), duplicate_data) {
		// set up
		auto container = container_t();
		container.add_data(data1.id, data1);

		// exercise and verify
		BOOST_CHECK_THROW(container.update_data(duplicate_data.id, duplicate_data), unique_variable_duplication_error);
	}

	////////////////////////////////
	// is_exist_data 
	////////////////////////////////

	BOOST_AUTO_TEST_CASE(test_is_exist_for_exist_data) {
		// set up
		auto container = container_t();
		container.add_data(data1.id, data1);

		// exercise
		const auto result = container.is_data_exist(data1.id);

		// verify
		BOOST_CHECK_EQUAL(result, true);
	}

	BOOST_AUTO_TEST_CASE(test_is_exist_for_not_exist_data) {
		// set up
		auto container = container_t();
		container.add_data(data1.id, data1);

		// exercise
		const auto result = container.is_data_exist(data2.id);

		// verify
		BOOST_CHECK_EQUAL(result, false);
	}

	////////////////////////////////
	// remove_data 
	////////////////////////////////

	BOOST_AUTO_TEST_CASE(test_remove) {
		// set up
		auto container = container_t();
		container.add_data(data1.id, data1);
		container.add_data(data2.id, data2);

		// exercise
		container.remove_data(data1.id);
		const auto actual1 = container.is_data_exist(data1.id);
		const auto actual2 = container.is_data_exist(data2.id);

		// verify
		BOOST_CHECK_EQUAL(actual1, false);
		BOOST_CHECK_EQUAL(actual2, true);
	}

	// Make not error and return result by bool?
	//BOOST_AUTO_TEST_CASE(test_remove_not_exist_data) {
	//	// set up
	//	auto container = container_t();

	//	// exercise and verify
	//	BOOST_CHECK_THROW(container.remove_data(data1.id), std::out_of_range);
	//}

	////////////////////////////////
	// size 
	////////////////////////////////

	BOOST_AUTO_TEST_CASE(test_size_for_no_data) {
		// set up
		const auto container = container_t();

		// exercise
		const auto result = container.size();

		// verify
		BOOST_CHECK_EQUAL(result, 0);
	}

	BOOST_AUTO_TEST_CASE(test_size_for_one_data) {
		// set up
		auto container = container_t();
		container.add_data(data1.id, data1);

		// exercise
		const auto result = container.size();

		// verify
		BOOST_CHECK_EQUAL(result, 1);
	}

BOOST_AUTO_TEST_SUITE_END()