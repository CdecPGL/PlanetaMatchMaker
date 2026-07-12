#include <boost/test/unit_test.hpp>

#include "protocol_test_support.hpp"

namespace {
	using namespace pgl::test;
}

BOOST_AUTO_TEST_SUITE(join_room_protocol_test)
	BOOST_AUTO_TEST_CASE(test_join_room_request_replies_host_endpoint_and_disconnects) {
		protocol_context context;
		mark_authenticated(context);
		auto room = make_room(1, {u8"host", 1}, public_open_room, {}, 2, 1);
		room.game_host_external_id[0] = 99;
		context.server_data.get_room_data_container().add_or_update(room);
		const pgl::join_room_request_message request{
			1,
			pgl::game_host_connection_establish_mode::builtin,
			{}
		};
		protocol_handler_run handler(context, pgl::message_type::join_room);

		write_packed(context.client_socket, pgl::request_message_header{pgl::message_type::join_room}, request);
		const auto reply_header = read_packed<pgl::reply_message_header>(context.client_socket);
		const auto reply = read_packed<pgl::join_room_reply_message>(context.client_socket);
		const auto exception = handler.wait();

		BOOST_CHECK(is_intended_disconnect(exception));
		BOOST_CHECK(reply_header.error_code == pgl::message_error_code::ok);
		BOOST_CHECK(reply.game_host_endpoint == room.game_host_endpoint);
		BOOST_CHECK_EQUAL(reply.game_host_external_id[0], 99);
		BOOST_CHECK_EQUAL(context.server_data.get_room_data_container().get(1).current_player_count, 2);
	}

	BOOST_AUTO_TEST_CASE(test_join_room_request_replies_password_error_without_body) {
		protocol_context context;
		mark_authenticated(context);
		context.server_data.get_room_data_container().add_or_update(make_room(1, {u8"host", 1},
			pgl::room_setting_flag::open_room, u8"secret"));
		const pgl::join_room_request_message request{
			1,
			pgl::game_host_connection_establish_mode::builtin,
			u8"wrong"
		};
		protocol_handler_run handler(context, pgl::message_type::join_room);

		write_packed(context.client_socket, pgl::request_message_header{pgl::message_type::join_room}, request);
		const auto reply_header = read_packed<pgl::reply_message_header>(context.client_socket);
		const auto exception = handler.wait();

		BOOST_CHECK(!exception);
		BOOST_CHECK(reply_header.message_type == pgl::message_type::join_room);
		BOOST_CHECK(reply_header.error_code == pgl::message_error_code::room_password_wrong);
		BOOST_CHECK_EQUAL(context.server_data.get_room_data_container().get(1).current_player_count, 1);
	}

	BOOST_AUTO_TEST_CASE(test_join_room_request_accepts_private_room_with_correct_password) {
		protocol_context context;
		mark_authenticated(context);
		auto room = make_room(1, {u8"host", 1}, pgl::room_setting_flag::open_room, u8"secret", 2, 1);
		context.server_data.get_room_data_container().add_or_update(room);
		const pgl::join_room_request_message request{
			1,
			pgl::game_host_connection_establish_mode::builtin,
			u8"secret"
		};
		protocol_handler_run handler(context, pgl::message_type::join_room);

		write_packed(context.client_socket, pgl::request_message_header{pgl::message_type::join_room}, request);
		const auto reply_header = read_packed<pgl::reply_message_header>(context.client_socket);
		const auto reply = read_packed<pgl::join_room_reply_message>(context.client_socket);
		const auto exception = handler.wait();

		BOOST_CHECK(is_intended_disconnect(exception));
		BOOST_CHECK(reply_header.error_code == pgl::message_error_code::ok);
		BOOST_CHECK(reply.game_host_endpoint == room.game_host_endpoint);
	}

	BOOST_AUTO_TEST_CASE(test_join_room_request_ignores_password_for_public_room) {
		protocol_context context;
		mark_authenticated(context);
		auto room = make_room(1, {u8"host", 1}, public_open_room, {}, 2, 1);
		context.server_data.get_room_data_container().add_or_update(room);
		const pgl::join_room_request_message request{
			1,
			pgl::game_host_connection_establish_mode::builtin,
			u8"wrong"
		};
		protocol_handler_run handler(context, pgl::message_type::join_room);

		write_packed(context.client_socket, pgl::request_message_header{pgl::message_type::join_room}, request);
		const auto reply_header = read_packed<pgl::reply_message_header>(context.client_socket);
		const auto reply = read_packed<pgl::join_room_reply_message>(context.client_socket);
		const auto exception = handler.wait();

		BOOST_CHECK(is_intended_disconnect(exception));
		BOOST_CHECK(reply_header.error_code == pgl::message_error_code::ok);
		BOOST_CHECK(reply.game_host_endpoint == room.game_host_endpoint);
	}

	BOOST_AUTO_TEST_CASE(test_join_room_request_replies_room_not_found) {
		protocol_context context;
		mark_authenticated(context);
		const pgl::join_room_request_message request{
			404,
			pgl::game_host_connection_establish_mode::builtin,
			{}
		};
		protocol_handler_run handler(context, pgl::message_type::join_room);

		write_packed(context.client_socket, pgl::request_message_header{pgl::message_type::join_room}, request);
		const auto reply_header = read_packed<pgl::reply_message_header>(context.client_socket);
		const auto exception = handler.wait();

		BOOST_CHECK(!exception);
		BOOST_CHECK(reply_header.error_code == pgl::message_error_code::room_not_found);
		expect_no_more_reply_data(context.client_socket);
	}

	BOOST_AUTO_TEST_CASE(test_join_room_request_replies_permission_denied_for_closed_room) {
		protocol_context context;
		mark_authenticated(context);
		context.server_data.get_room_data_container().add_or_update(make_room(1, {u8"host", 1},
			pgl::room_setting_flag::public_room));
		const pgl::join_room_request_message request{
			1,
			pgl::game_host_connection_establish_mode::builtin,
			{}
		};
		protocol_handler_run handler(context, pgl::message_type::join_room);

		write_packed(context.client_socket, pgl::request_message_header{pgl::message_type::join_room}, request);
		const auto reply_header = read_packed<pgl::reply_message_header>(context.client_socket);
		const auto exception = handler.wait();

		BOOST_CHECK(!exception);
		BOOST_CHECK(reply_header.error_code == pgl::message_error_code::room_permission_denied);
		expect_no_more_reply_data(context.client_socket);
	}

	BOOST_AUTO_TEST_CASE(test_join_room_request_replies_room_full) {
		protocol_context context;
		mark_authenticated(context);
		context.server_data.get_room_data_container().add_or_update(make_room(1, {u8"host", 1},
			public_open_room, {}, 2, 2));
		const pgl::join_room_request_message request{
			1,
			pgl::game_host_connection_establish_mode::builtin,
			{}
		};
		protocol_handler_run handler(context, pgl::message_type::join_room);

		write_packed(context.client_socket, pgl::request_message_header{pgl::message_type::join_room}, request);
		const auto reply_header = read_packed<pgl::reply_message_header>(context.client_socket);
		const auto exception = handler.wait();

		BOOST_CHECK(!exception);
		BOOST_CHECK(reply_header.error_code == pgl::message_error_code::room_full);
		expect_no_more_reply_data(context.client_socket);
	}

	BOOST_AUTO_TEST_CASE(test_join_room_request_replies_connection_establish_mode_mismatch) {
		protocol_context context;
		mark_authenticated(context);
		auto room = make_room(1, {u8"host", 1});
		room.game_host_connection_establish_mode = pgl::game_host_connection_establish_mode::steam;
		context.server_data.get_room_data_container().add_or_update(room);
		const pgl::join_room_request_message request{
			1,
			pgl::game_host_connection_establish_mode::builtin,
			{}
		};
		protocol_handler_run handler(context, pgl::message_type::join_room);

		write_packed(context.client_socket, pgl::request_message_header{pgl::message_type::join_room}, request);
		const auto reply_header = read_packed<pgl::reply_message_header>(context.client_socket);
		const auto exception = handler.wait();

		BOOST_CHECK(!exception);
		BOOST_CHECK(reply_header.error_code == pgl::message_error_code::room_connection_establish_mode_mismatch);
		expect_no_more_reply_data(context.client_socket);
	}

BOOST_AUTO_TEST_SUITE_END()
