#include <atomic>
#include <thread>
#include <vector>

#include <boost/test/unit_test.hpp>

#include "../PlanetaMatchMakerServer/source/room/room_data_container.hpp"

using namespace pgl;

namespace {
	constexpr auto public_open_room = room_setting_flag::public_room | room_setting_flag::open_room;

	endpoint make_endpoint(const port_number_type port_number) {
		auto endpoint = pgl::endpoint();
		endpoint.port_number = port_number;
		return endpoint;
	}

	room_data make_room(const room_id_t room_id, const player_tag_t host_tag, const uint8_t max_player_count,
		const uint8_t current_player_count, const room_setting_flag setting_flags = public_open_room) {
		return {
			room_id,
			{ u8"host", host_tag },
			setting_flags,
			{},
			max_player_count,
			datetime(2024, 1, 1),
			make_endpoint(1234),
			game_host_connection_establish_mode::builtin,
			make_endpoint(5678),
			{},
			current_player_count
		};
	}
}

BOOST_AUTO_TEST_SUITE(room_data_container_test)

	BOOST_AUTO_TEST_CASE(test_try_reserve_player_for_join_accepts_and_increments_current_player_count) {
		// set up
		auto container = room_data_container();
		container.add_or_update(make_room(1, 1, 2, 1));

		// exercise
		const auto result = container.try_reserve_player_for_join(1, game_host_connection_establish_mode::builtin, {});

		// verify
		BOOST_CHECK(result.result == room_data_container::join_room_result::accepted);
		BOOST_REQUIRE(result.room.has_value());
		BOOST_CHECK_EQUAL(result.room->current_player_count, 2);
		BOOST_CHECK_EQUAL(container.get(1).current_player_count, 2);
	}

	BOOST_AUTO_TEST_CASE(test_try_reserve_player_for_join_rejects_full_room_without_incrementing) {
		// set up
		auto container = room_data_container();
		container.add_or_update(make_room(1, 1, 2, 2));

		// exercise
		const auto result = container.try_reserve_player_for_join(1, game_host_connection_establish_mode::builtin, {});

		// verify
		BOOST_CHECK(result.result == room_data_container::join_room_result::room_full);
		BOOST_REQUIRE(result.room.has_value());
		BOOST_CHECK_EQUAL(result.room->current_player_count, 2);
		BOOST_CHECK_EQUAL(container.get(1).current_player_count, 2);
	}

	BOOST_AUTO_TEST_CASE(test_host_report_does_not_drop_in_flight_join_reservation) {
		// set up
		auto container = room_data_container();
		container.add_or_update(make_room(1, 1, 2, 1));
		const auto reserve_result = container.try_reserve_player_for_join(1,
			game_host_connection_establish_mode::builtin, {});
		BOOST_REQUIRE(reserve_result.result == room_data_container::join_room_result::accepted);

		// exercise
		const auto update_result = container.try_update_with_host_reported_current_player_count(1, true, 1,
			[](auto&) {});
		const auto next_join_result = container.try_reserve_player_for_join(1,
			game_host_connection_establish_mode::builtin, {});

		// verify
		BOOST_REQUIRE(update_result.has_value());
		BOOST_CHECK_EQUAL(update_result->current_player_count, 2);
		BOOST_CHECK_EQUAL(container.get(1).current_player_count, 2);
		BOOST_CHECK(next_join_result.result == room_data_container::join_room_result::room_full);
	}

	BOOST_AUTO_TEST_CASE(test_host_report_confirms_in_flight_join_reservation) {
		// set up
		auto container = room_data_container();
		container.add_or_update(make_room(1, 1, 4, 1));
		const auto reserve_result = container.try_reserve_player_for_join(1,
			game_host_connection_establish_mode::builtin, {});
		BOOST_REQUIRE(reserve_result.result == room_data_container::join_room_result::accepted);

		// exercise
		const auto confirmed_result = container.try_update_with_host_reported_current_player_count(1, true, 2,
			[](auto&) {});
		const auto reduced_result = container.try_update_with_host_reported_current_player_count(1, true, 1,
			[](auto&) {});

		// verify
		BOOST_REQUIRE(confirmed_result.has_value());
		BOOST_CHECK_EQUAL(confirmed_result->current_player_count, 2);
		BOOST_REQUIRE(reduced_result.has_value());
		BOOST_CHECK_EQUAL(reduced_result->current_player_count, 1);
		BOOST_CHECK_EQUAL(container.get(1).current_player_count, 1);
	}

	BOOST_AUTO_TEST_CASE(test_concurrent_try_reserve_player_for_join_does_not_exceed_max_player_count) {
		// set up
		auto container = room_data_container();
		constexpr auto max_player_count = uint8_t{4};
		container.add_or_update(make_room(1, 1, max_player_count, 1));
		constexpr auto thread_count = 8u;
		std::atomic<bool> can_start = false;
		std::atomic<unsigned> accepted_count = 0;
		std::atomic<unsigned> full_count = 0;
		std::vector<std::thread> threads;
		threads.reserve(thread_count);

		// exercise
		for (auto thread_index = 0u; thread_index < thread_count; ++thread_index) {
			threads.emplace_back([&] {
				while (!can_start.load(std::memory_order_acquire)) { std::this_thread::yield(); }

				const auto result = container.try_reserve_player_for_join(1,
					game_host_connection_establish_mode::builtin, {});
				if (result.result == room_data_container::join_room_result::accepted) { ++accepted_count; }
				if (result.result == room_data_container::join_room_result::room_full) { ++full_count; }
			});
		}

		can_start.store(true, std::memory_order_release);
		for (auto& thread : threads) { thread.join(); }

		// verify
		BOOST_CHECK_EQUAL(accepted_count.load(), max_player_count - 1);
		BOOST_CHECK_EQUAL(full_count.load(), thread_count - accepted_count.load());
		BOOST_CHECK_EQUAL(container.get(1).current_player_count, max_player_count);
	}

	BOOST_AUTO_TEST_CASE(test_search_with_total_returns_match_count_and_total_count_from_one_snapshot) {
		// set up
		auto container = room_data_container();
		container.add_or_update(make_room(1, 1, 4, 1));
		container.add_or_update(make_room(2, 2, 4, 1, room_setting_flag::public_room));

		// exercise
		const auto result = container.search_with_total(room_data_sort_kind::name_ascending,
			room_search_target_flag::public_room | room_search_target_flag::open_room, {});

		// verify
		BOOST_CHECK_EQUAL(result.total_room_count, 2);
		BOOST_REQUIRE_EQUAL(result.data.size(), 1);
		BOOST_CHECK_EQUAL(result.data.front().room_id, 1);
	}

BOOST_AUTO_TEST_SUITE_END()
