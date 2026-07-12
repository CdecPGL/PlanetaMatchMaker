#include <boost/test/unit_test.hpp>

#include "client/player_name_container.hpp"

BOOST_AUTO_TEST_SUITE(player_name_container_test)
	BOOST_AUTO_TEST_CASE(test_assign_player_name_assigns_first_tag_for_new_name) {
		pgl::player_name_container container;

		const auto full_name = container.assign_player_name(u8"player");

		BOOST_CHECK(full_name.name == u8"player");
		BOOST_CHECK_EQUAL(full_name.tag, 1);
		BOOST_CHECK(container.is_player_exist(full_name));
	}

	BOOST_AUTO_TEST_CASE(test_assign_player_name_assigns_next_tag_for_same_name) {
		pgl::player_name_container container;

		const auto first = container.assign_player_name(u8"player");
		const auto second = container.assign_player_name(u8"player");

		BOOST_CHECK(first.name == second.name);
		BOOST_CHECK_EQUAL(first.tag, 1);
		BOOST_CHECK_EQUAL(second.tag, 2);
		BOOST_CHECK(container.is_player_exist(first));
		BOOST_CHECK(container.is_player_exist(second));
	}

	BOOST_AUTO_TEST_CASE(test_assign_player_name_uses_independent_tag_sequence_per_name) {
		pgl::player_name_container container;

		const auto first = container.assign_player_name(u8"alice");
		const auto second = container.assign_player_name(u8"bob");

		BOOST_CHECK(first.name == u8"alice");
		BOOST_CHECK(second.name == u8"bob");
		BOOST_CHECK_EQUAL(first.tag, 1);
		BOOST_CHECK_EQUAL(second.tag, 1);
	}

	BOOST_AUTO_TEST_CASE(test_remove_player_name_removes_only_target_full_name) {
		pgl::player_name_container container;
		const auto first = container.assign_player_name(u8"player");
		const auto second = container.assign_player_name(u8"player");

		container.remove_player_name(first);

		BOOST_CHECK(!container.is_player_exist(first));
		BOOST_CHECK(container.is_player_exist(second));
	}

	BOOST_AUTO_TEST_CASE(test_remove_player_name_erases_empty_name_entry) {
		pgl::player_name_container container;
		const auto first = container.assign_player_name(u8"player");
		container.remove_player_name(first);

		const auto second = container.assign_player_name(u8"player");

		BOOST_CHECK_EQUAL(second.tag, 1);
		BOOST_CHECK(container.is_player_exist(second));
	}

	BOOST_AUTO_TEST_CASE(test_remove_player_name_rejects_unknown_name) {
		pgl::player_name_container container;

		BOOST_CHECK_THROW(container.remove_player_name({u8"missing", 1}), pgl::player_name_error);
	}

	BOOST_AUTO_TEST_CASE(test_remove_player_name_rejects_unknown_tag) {
		pgl::player_name_container container;
		container.assign_player_name(u8"player");

		BOOST_CHECK_THROW(container.remove_player_name({u8"player", 2}), pgl::player_name_error);
	}

	BOOST_AUTO_TEST_CASE(test_is_player_exist_returns_false_for_unknown_name_or_tag) {
		pgl::player_name_container container;
		const auto full_name = container.assign_player_name(u8"player");

		BOOST_CHECK(container.is_player_exist(full_name));
		BOOST_CHECK(!container.is_player_exist({u8"missing", 1}));
		BOOST_CHECK(!container.is_player_exist({u8"player", 2}));
	}

BOOST_AUTO_TEST_SUITE_END()
