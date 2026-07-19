#include <boost/test/unit_test.hpp>

#include <stdexcept>

#include "session/session_data.hpp"

BOOST_AUTO_TEST_SUITE(session_data_test)
	BOOST_AUTO_TEST_CASE(test_session_number_is_empty_by_default) {
		const pgl::session_data session_data;

		BOOST_CHECK(!session_data.session_number().has_value());
	}

	BOOST_AUTO_TEST_CASE(test_set_session_number_stores_value) {
		pgl::session_data session_data;

		session_data.set_session_number(42);

		BOOST_REQUIRE(session_data.session_number().has_value());
		BOOST_CHECK_EQUAL(*session_data.session_number(), 42);
	}

	BOOST_AUTO_TEST_CASE(test_set_session_number_rejects_reassignment) {
		pgl::session_data session_data;
		session_data.set_session_number(42);

		BOOST_CHECK_EXCEPTION(session_data.set_session_number(43), std::runtime_error,
			[](const auto&) { return true; });
	}

	BOOST_AUTO_TEST_CASE(test_hosting_room_id_is_not_set_by_default) {
		const pgl::session_data session_data;

		BOOST_CHECK(!session_data.is_hosting_room());
		BOOST_CHECK_THROW(session_data.hosting_room_id(), std::runtime_error);
	}

	BOOST_AUTO_TEST_CASE(test_set_hosting_room_id_stores_value) {
		pgl::session_data session_data;

		session_data.set_hosting_room_id(123);

		BOOST_CHECK(session_data.is_hosting_room());
		BOOST_CHECK_EQUAL(session_data.hosting_room_id(), pgl::room_id_t{123});
	}

	BOOST_AUTO_TEST_CASE(test_set_hosting_room_id_rejects_reassignment) {
		pgl::session_data session_data;
		session_data.set_hosting_room_id(123);

		BOOST_CHECK_THROW(session_data.set_hosting_room_id(456), std::runtime_error);
	}

	BOOST_AUTO_TEST_CASE(test_delete_hosting_room_id_clears_matching_room) {
		pgl::session_data session_data;
		session_data.set_hosting_room_id(123);

		session_data.delete_hosting_room_id(123);

		BOOST_CHECK(!session_data.is_hosting_room());
		BOOST_CHECK_THROW(session_data.hosting_room_id(), std::runtime_error);
	}

	BOOST_AUTO_TEST_CASE(test_delete_hosting_room_id_rejects_missing_room) {
		pgl::session_data session_data;

		BOOST_CHECK_THROW(session_data.delete_hosting_room_id(123), std::runtime_error);
	}

	BOOST_AUTO_TEST_CASE(test_delete_hosting_room_id_rejects_different_room_id) {
		pgl::session_data session_data;
		session_data.set_hosting_room_id(123);

		BOOST_CHECK_THROW(session_data.delete_hosting_room_id(456), std::runtime_error);
		BOOST_CHECK(session_data.is_hosting_room());
		BOOST_CHECK_EQUAL(session_data.hosting_room_id(), pgl::room_id_t{123});
	}

	BOOST_AUTO_TEST_CASE(test_set_remote_endpoint_stores_value) {
		pgl::session_data session_data;
		pgl::endpoint endpoint{};
		endpoint.port_number = 57000;
		endpoint.ip_address[15] = 1;

		session_data.set_remote_endpoint(endpoint);

		BOOST_CHECK(session_data.remote_endpoint() == endpoint);
	}

	BOOST_AUTO_TEST_CASE(test_set_client_player_name_stores_value) {
		pgl::session_data session_data;
		const pgl::player_full_name player_full_name{u8"player", 42};

		session_data.set_client_player_name(player_full_name);

		BOOST_CHECK(session_data.client_player_name() == player_full_name);
	}

	BOOST_AUTO_TEST_CASE(test_authenticated_is_false_by_default) {
		const pgl::session_data session_data;

		BOOST_CHECK(!session_data.is_authenticated());
	}

	BOOST_AUTO_TEST_CASE(test_set_authenticated_marks_session_authenticated) {
		pgl::session_data session_data;

		session_data.set_authenticated();

		BOOST_CHECK(session_data.is_authenticated());
		BOOST_CHECK(!session_data.identity().has_value());
	}

	BOOST_AUTO_TEST_CASE(test_set_authenticated_with_identity_stores_only_provider_user_id) {
		pgl::session_data session_data;
		const pgl::authenticated_identity identity{
			pgl::authentication_provider_user_id_t{u8"76561198000000000"}
		};

		session_data.set_authenticated(identity);

		BOOST_REQUIRE(session_data.identity().has_value());
		BOOST_CHECK(session_data.identity()->authentication_provider_user_id ==
			identity.authentication_provider_user_id);
	}

	BOOST_AUTO_TEST_CASE(test_set_authenticated_rejects_empty_provider_user_id) {
		pgl::session_data session_data;

		BOOST_CHECK_THROW(session_data.set_authenticated(pgl::authenticated_identity{}), std::invalid_argument);
		BOOST_CHECK(!session_data.is_authenticated());
	}

	BOOST_AUTO_TEST_CASE(test_set_authenticated_rejects_embedded_nul_provider_user_id) {
		pgl::session_data session_data;
		pgl::authenticated_identity identity{};
		identity.authentication_provider_user_id[0] = 'a';
		identity.authentication_provider_user_id[2] = 'b';

		BOOST_CHECK_THROW(session_data.set_authenticated(identity), std::invalid_argument);
		BOOST_CHECK(!session_data.is_authenticated());
	}

	BOOST_AUTO_TEST_CASE(test_set_authenticated_rejects_invalid_utf8_provider_user_id) {
		pgl::session_data session_data;
		pgl::authenticated_identity identity{};
		identity.authentication_provider_user_id[0] = 0xff;

		BOOST_CHECK_THROW(session_data.set_authenticated(identity), std::invalid_argument);
		BOOST_CHECK(!session_data.is_authenticated());
	}

	BOOST_AUTO_TEST_CASE(test_set_authenticated_rejects_reassignment) {
		pgl::session_data session_data;
		session_data.set_authenticated();

		BOOST_CHECK_THROW(session_data.set_authenticated(), std::runtime_error);
	}

BOOST_AUTO_TEST_SUITE_END()
