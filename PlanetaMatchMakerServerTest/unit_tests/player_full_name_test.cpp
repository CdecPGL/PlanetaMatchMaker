#include <boost/test/unit_test.hpp>

#include "../../PlanetaMatchMakerServer/source/client/player_full_name.hpp"

BOOST_AUTO_TEST_SUITE(player_full_name_test)
	BOOST_AUTO_TEST_CASE(test_generate_full_name_combines_name_and_tag) {
		const pgl::player_full_name player_full_name{u8"player", 42};

		BOOST_CHECK_EQUAL(player_full_name.generate_full_name(), "player#42");
	}

	BOOST_AUTO_TEST_CASE(test_is_name_and_tag_assigned_reflects_empty_values) {
		const pgl::player_full_name assigned{u8"player", 42};
		const pgl::player_full_name unassigned{u8"", 0};

		BOOST_CHECK(assigned.is_name_assigned());
		BOOST_CHECK(assigned.is_tag_assigned());
		BOOST_CHECK(!unassigned.is_name_assigned());
		BOOST_CHECK(!unassigned.is_tag_assigned());
	}

	BOOST_AUTO_TEST_CASE(test_equal_operator_compares_name_and_tag) {
		const pgl::player_full_name player_full_name{u8"player", 42};

		BOOST_CHECK(player_full_name == (pgl::player_full_name{u8"player", 42}));
		BOOST_CHECK(!(player_full_name == (pgl::player_full_name{u8"player", 43})));
		BOOST_CHECK(!(player_full_name == (pgl::player_full_name{u8"other", 42})));
	}

	BOOST_AUTO_TEST_CASE(test_not_equal_operator_is_inverse_of_equal_operator) {
		const pgl::player_full_name player_full_name{u8"player", 42};

		BOOST_CHECK(!(player_full_name != (pgl::player_full_name{u8"player", 42})));
		BOOST_CHECK(player_full_name != (pgl::player_full_name{u8"player", 43}));
	}

BOOST_AUTO_TEST_SUITE_END()
