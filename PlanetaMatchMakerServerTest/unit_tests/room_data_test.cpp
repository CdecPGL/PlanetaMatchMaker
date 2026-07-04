#include <boost/test/unit_test.hpp>

#include <algorithm>
#include <iterator>
#include <sstream>
#include <vector>

#include "../../PlanetaMatchMakerServer/source/room/room_data.hpp"

namespace {
	constexpr auto public_open_room = pgl::room_setting_flag::public_room | pgl::room_setting_flag::open_room;
	constexpr auto private_open_room = pgl::room_setting_flag::open_room;
	constexpr auto public_closed_room = pgl::room_setting_flag::public_room;
	constexpr auto private_closed_room = pgl::room_setting_flag::none;

	pgl::endpoint make_endpoint(const pgl::port_number_type port_number) {
		auto endpoint = pgl::endpoint();
		endpoint.port_number = port_number;
		return endpoint;
	}

	pgl::room_data make_room(const pgl::room_id_t room_id, const char8_t* name, const pgl::player_tag_t tag,
		const pgl::room_setting_flag setting_flags = public_open_room,
		const pgl::datetime create_datetime = pgl::datetime(2024, 1, 1)) {
		return {
			room_id,
			{name, tag},
			setting_flags,
			{},
			4,
			create_datetime,
			make_endpoint(57000),
			pgl::game_host_connection_establish_mode::builtin,
			make_endpoint(57001),
			{},
			1
		};
	}

	std::vector<pgl::room_id_t> extract_room_ids(const std::vector<pgl::room_data>& rooms) {
		std::vector<pgl::room_id_t> room_ids;
		room_ids.reserve(rooms.size());
		std::ranges::transform(rooms, std::back_inserter(room_ids), [](const auto& room) {
			return room.room_id;
		});
		return room_ids;
	}
}

BOOST_AUTO_TEST_SUITE(room_data_test)
	BOOST_AUTO_TEST_CASE(test_name_ascending_compare_sorts_by_name_then_tag) {
		std::vector rooms{
			make_room(1, u8"bob", 2),
			make_room(2, u8"alice", 1),
			make_room(3, u8"bob", 1),
		};
		const auto compare = pgl::get_room_data_compare_function(
			pgl::room_data_sort_kind::name_ascending, {});

		std::ranges::sort(rooms, compare);

		const auto actual = extract_room_ids(rooms);
		const std::vector<pgl::room_id_t> expected{2, 3, 1};
		BOOST_CHECK_EQUAL_COLLECTIONS(actual.begin(), actual.end(), expected.begin(), expected.end());
	}

	BOOST_AUTO_TEST_CASE(test_name_descending_compare_sorts_by_name_then_tag_descending) {
		std::vector rooms{
			make_room(1, u8"bob", 2),
			make_room(2, u8"alice", 1),
			make_room(3, u8"bob", 1),
		};
		const auto compare = pgl::get_room_data_compare_function(
			pgl::room_data_sort_kind::name_descending, {});

		std::ranges::sort(rooms, compare);

		const auto actual = extract_room_ids(rooms);
		const std::vector<pgl::room_id_t> expected{1, 3, 2};
		BOOST_CHECK_EQUAL_COLLECTIONS(actual.begin(), actual.end(), expected.begin(), expected.end());
	}

	BOOST_AUTO_TEST_CASE(test_create_datetime_compare_sorts_ascending_and_descending) {
		std::vector ascending_rooms{
			make_room(1, u8"one", 1, public_open_room, pgl::datetime(2024, 1, 2)),
			make_room(2, u8"two", 1, public_open_room, pgl::datetime(2024, 1, 1)),
		};
		auto descending_rooms = ascending_rooms;

		std::ranges::sort(ascending_rooms, pgl::get_room_data_compare_function(
			pgl::room_data_sort_kind::create_datetime_ascending, {}));
		std::ranges::sort(descending_rooms, pgl::get_room_data_compare_function(
			pgl::room_data_sort_kind::create_datetime_descending, {}));

		const auto ascending = extract_room_ids(ascending_rooms);
		const auto descending = extract_room_ids(descending_rooms);
		const std::vector<pgl::room_id_t> expected_ascending{2, 1};
		const std::vector<pgl::room_id_t> expected_descending{1, 2};
		BOOST_CHECK_EQUAL_COLLECTIONS(ascending.begin(), ascending.end(),
			expected_ascending.begin(), expected_ascending.end());
		BOOST_CHECK_EQUAL_COLLECTIONS(descending.begin(), descending.end(),
			expected_descending.begin(), expected_descending.end());
	}

	BOOST_AUTO_TEST_CASE(test_compare_function_rejects_invalid_sort_kind) {
		BOOST_CHECK_THROW(pgl::get_room_data_compare_function(static_cast<pgl::room_data_sort_kind>(255), {}),
			std::out_of_range);
	}

	BOOST_AUTO_TEST_CASE(test_compare_function_prioritizes_exact_search_name) {
		const auto compare = pgl::get_room_data_compare_function(
			pgl::room_data_sort_kind::name_ascending, {u8"bob", 0});
		const auto exact_match = make_room(1, u8"bob", 2);
		const auto partial_match = make_room(2, u8"bobcat", 1);

		BOOST_CHECK(compare(exact_match, partial_match));
		BOOST_CHECK(!compare(partial_match, exact_match));
	}

	BOOST_AUTO_TEST_CASE(test_filter_function_matches_public_and_open_flags) {
		const auto filter = pgl::get_room_data_filter_function(
			pgl::room_search_target_flag::public_room | pgl::room_search_target_flag::open_room, {});

		BOOST_CHECK(filter(make_room(1, u8"public-open", 1, public_open_room)));
		BOOST_CHECK(!filter(make_room(2, u8"private-open", 1, private_open_room)));
		BOOST_CHECK(!filter(make_room(3, u8"public-closed", 1, public_closed_room)));
		BOOST_CHECK(!filter(make_room(4, u8"private-closed", 1, private_closed_room)));
	}

	BOOST_AUTO_TEST_CASE(test_filter_function_matches_private_and_closed_flags) {
		const auto filter = pgl::get_room_data_filter_function(
			pgl::room_search_target_flag::private_room | pgl::room_search_target_flag::closed_room, {});

		BOOST_CHECK(!filter(make_room(1, u8"public-open", 1, public_open_room)));
		BOOST_CHECK(!filter(make_room(2, u8"private-open", 1, private_open_room)));
		BOOST_CHECK(!filter(make_room(3, u8"public-closed", 1, public_closed_room)));
		BOOST_CHECK(filter(make_room(4, u8"private-closed", 1, private_closed_room)));
	}

	BOOST_AUTO_TEST_CASE(test_filter_function_matches_search_name_substring) {
		const auto all_room_statuses = pgl::room_search_target_flag::public_room |
			pgl::room_search_target_flag::private_room |
			pgl::room_search_target_flag::open_room |
			pgl::room_search_target_flag::closed_room;
		const auto filter = pgl::get_room_data_filter_function(all_room_statuses, {u8"lic", 0});

		BOOST_CHECK(filter(make_room(1, u8"alice", 1, public_open_room)));
		BOOST_CHECK(!filter(make_room(2, u8"bob", 1, public_open_room)));
	}

	BOOST_AUTO_TEST_CASE(test_filter_function_matches_search_tag) {
		const auto all_room_statuses = pgl::room_search_target_flag::public_room |
			pgl::room_search_target_flag::private_room |
			pgl::room_search_target_flag::open_room |
			pgl::room_search_target_flag::closed_room;
		const auto filter = pgl::get_room_data_filter_function(all_room_statuses, {u8"", 42});

		BOOST_CHECK(filter(make_room(1, u8"alice", 42, public_open_room)));
		BOOST_CHECK(!filter(make_room(2, u8"alice", 43, public_open_room)));
	}

	BOOST_AUTO_TEST_CASE(test_filter_function_matches_search_name_and_tag_together) {
		const auto all_room_statuses = pgl::room_search_target_flag::public_room |
			pgl::room_search_target_flag::private_room |
			pgl::room_search_target_flag::open_room |
			pgl::room_search_target_flag::closed_room;
		const auto filter = pgl::get_room_data_filter_function(all_room_statuses, {u8"ali", 42});

		BOOST_CHECK(filter(make_room(1, u8"alice", 42, public_open_room)));
		BOOST_CHECK(!filter(make_room(2, u8"alice", 43, public_open_room)));
		BOOST_CHECK(!filter(make_room(3, u8"bob", 42, public_open_room)));
	}

	BOOST_AUTO_TEST_CASE(test_room_data_stream_operator_outputs_id_and_host_full_name) {
		const auto room = make_room(123, u8"alice", 42);
		std::ostringstream stream;

		stream << room;

		BOOST_CHECK_EQUAL(stream.str(), "room(ID=123, host_player=alice#42)");
	}

BOOST_AUTO_TEST_SUITE_END()
