#include <boost/test/unit_test.hpp>

#include "protocol_test_support.hpp"

namespace {
	using namespace pgl::test;
}

BOOST_AUTO_TEST_SUITE(create_room_protocol_test)
	BOOST_AUTO_TEST_CASE(test_create_room_request_creates_public_room_and_replies_room_id) {
		protocol_context context;
		context.session_data.set_authenticated();
		context.session_data.set_client_player_name({u8"host", 1});
		const pgl::create_room_request_message request{
			{},
			4,
			pgl::game_host_connection_establish_mode::builtin,
			57000,
			{}
		};
		protocol_handler_run handler(context, pgl::message_type::create_room);

		write_packed(context.client_socket, pgl::request_message_header{pgl::message_type::create_room}, request);
		const auto reply_header = read_packed<pgl::reply_message_header>(context.client_socket);
		const auto reply = read_packed<pgl::create_room_reply_message>(context.client_socket);
		const auto exception = handler.wait();
		const auto room = context.server_data.get_room_data_container().get(reply.room_id);

		BOOST_CHECK(!exception);
		BOOST_CHECK(reply_header.error_code == pgl::message_error_code::ok);
		BOOST_CHECK(context.session_data.is_hosting_room());
		BOOST_CHECK_EQUAL(context.session_data.hosting_room_id(), reply.room_id);
		BOOST_CHECK(room.host_player_full_name == (pgl::player_full_name{u8"host", 1}));
		BOOST_CHECK((room.setting_flags & pgl::room_setting_flag::public_room) == pgl::room_setting_flag::public_room);
		BOOST_CHECK((room.setting_flags & pgl::room_setting_flag::open_room) == pgl::room_setting_flag::open_room);
		BOOST_CHECK_EQUAL(room.game_host_endpoint.port_number, request.port_number);
	}

	BOOST_AUTO_TEST_CASE(test_create_room_request_creates_private_room_when_password_is_set) {
		protocol_context context;
		mark_authenticated(context);
		const pgl::create_room_request_message request{
			u8"secret",
			4,
			pgl::game_host_connection_establish_mode::builtin,
			57000,
			{}
		};
		protocol_handler_run handler(context, pgl::message_type::create_room);

		write_packed(context.client_socket, pgl::request_message_header{pgl::message_type::create_room}, request);
		const auto reply_header = read_packed<pgl::reply_message_header>(context.client_socket);
		const auto reply = read_packed<pgl::create_room_reply_message>(context.client_socket);
		const auto exception = handler.wait();
		const auto room = context.server_data.get_room_data_container().get(reply.room_id);

		BOOST_CHECK(!exception);
		BOOST_CHECK(reply_header.error_code == pgl::message_error_code::ok);
		BOOST_CHECK((room.setting_flags & pgl::room_setting_flag::public_room) == pgl::room_setting_flag::none);
		BOOST_CHECK((room.setting_flags & pgl::room_setting_flag::open_room) == pgl::room_setting_flag::open_room);
		BOOST_CHECK(room.password == request.password);
	}

	BOOST_AUTO_TEST_CASE(test_create_room_request_creates_external_service_room_without_port_validation) {
		protocol_context context;
		mark_authenticated(context);
		pgl::game_host_external_id_t external_id{};
		external_id[0] = 7;
		const pgl::create_room_request_message request{
			{},
			4,
			pgl::game_host_connection_establish_mode::steam,
			1,
			external_id
		};
		protocol_handler_run handler(context, pgl::message_type::create_room);

		write_packed(context.client_socket, pgl::request_message_header{pgl::message_type::create_room}, request);
		const auto reply_header = read_packed<pgl::reply_message_header>(context.client_socket);
		const auto reply = read_packed<pgl::create_room_reply_message>(context.client_socket);
		const auto exception = handler.wait();
		const auto room = context.server_data.get_room_data_container().get(reply.room_id);

		BOOST_CHECK(!exception);
		BOOST_CHECK(reply_header.error_code == pgl::message_error_code::ok);
		BOOST_CHECK(room.game_host_connection_establish_mode == pgl::game_host_connection_establish_mode::steam);
		BOOST_CHECK_EQUAL(room.game_host_external_id[0], 7);
	}

	BOOST_AUTO_TEST_CASE(test_create_room_request_replies_parameter_error_for_invalid_builtin_port) {
		protocol_context context;
		mark_authenticated(context);
		const pgl::create_room_request_message request{
			{},
			4,
			pgl::game_host_connection_establish_mode::builtin,
			49151,
			{}
		};
		protocol_handler_run handler(context, pgl::message_type::create_room);

		write_packed(context.client_socket, pgl::request_message_header{pgl::message_type::create_room}, request);
		const auto reply_header = read_packed<pgl::reply_message_header>(context.client_socket);
		const auto exception = handler.wait();

		BOOST_CHECK(!exception);
		BOOST_CHECK(reply_header.error_code == pgl::message_error_code::request_parameter_wrong);
		expect_no_more_reply_data(context.client_socket);
	}

	BOOST_AUTO_TEST_CASE(test_create_room_request_replies_parameter_error_for_invalid_max_player_count) {
		protocol_context context;
		mark_authenticated(context);
		const pgl::create_room_request_message request{
			{},
			0,
			pgl::game_host_connection_establish_mode::builtin,
			57000,
			{}
		};
		protocol_handler_run handler(context, pgl::message_type::create_room);

		write_packed(context.client_socket, pgl::request_message_header{pgl::message_type::create_room}, request);
		const auto reply_header = read_packed<pgl::reply_message_header>(context.client_socket);
		const auto exception = handler.wait();

		BOOST_CHECK(!exception);
		BOOST_CHECK(reply_header.error_code == pgl::message_error_code::request_parameter_wrong);
		expect_no_more_reply_data(context.client_socket);
	}

	BOOST_AUTO_TEST_CASE(test_create_room_request_replies_client_already_hosting_room) {
		protocol_context context;
		mark_authenticated(context);
		context.session_data.set_hosting_room_id(1);
		const pgl::create_room_request_message request{
			{},
			4,
			pgl::game_host_connection_establish_mode::builtin,
			57000,
			{}
		};
		protocol_handler_run handler(context, pgl::message_type::create_room);

		write_packed(context.client_socket, pgl::request_message_header{pgl::message_type::create_room}, request);
		const auto reply_header = read_packed<pgl::reply_message_header>(context.client_socket);
		const auto exception = handler.wait();

		BOOST_CHECK(!exception);
		BOOST_CHECK(reply_header.error_code == pgl::message_error_code::client_already_hosting_room);
		expect_no_more_reply_data(context.client_socket);
	}

	BOOST_AUTO_TEST_CASE(test_create_room_request_replies_room_count_exceeds_limit) {
		protocol_context context;
		mark_authenticated(context);
		context.setting.common.max_room_count = 1;
		context.server_data.get_room_data_container().add_or_update(make_room(1, {u8"other", 1}));
		const pgl::create_room_request_message request{
			{},
			4,
			pgl::game_host_connection_establish_mode::builtin,
			57000,
			{}
		};
		protocol_handler_run handler(context, pgl::message_type::create_room);

		write_packed(context.client_socket, pgl::request_message_header{pgl::message_type::create_room}, request);
		const auto reply_header = read_packed<pgl::reply_message_header>(context.client_socket);
		const auto exception = handler.wait();

		BOOST_CHECK(!exception);
		BOOST_CHECK(reply_header.error_code == pgl::message_error_code::room_count_exceeds_limit);
		expect_no_more_reply_data(context.client_socket);
	}

BOOST_AUTO_TEST_SUITE_END()
