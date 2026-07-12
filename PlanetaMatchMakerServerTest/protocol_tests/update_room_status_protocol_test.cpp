#include <boost/test/unit_test.hpp>

#include "protocol_test_support.hpp"

namespace {
	using namespace pgl::test;
}

BOOST_AUTO_TEST_SUITE(update_room_status_protocol_test)
	BOOST_AUTO_TEST_CASE(test_update_room_status_notice_closes_room_without_reply) {
		protocol_context context;
		mark_authenticated(context);
		auto room = make_room(1, {u8"host", 1});
		room.host_endpoint = pgl::endpoint::make_from_boost_endpoint(context.server_connection.remote_endpoint());
		context.server_data.get_room_data_container().add_or_update(room);
		const pgl::update_room_status_notice_message request{
			1,
			pgl::update_room_status_notice_message::status::close,
			true,
			1
		};
		protocol_handler_run handler(context, pgl::message_type::update_room_status);

		write_packed(context.client_socket, pgl::request_message_header{pgl::message_type::update_room_status}, request);
		const auto exception = handler.wait();
		const auto updated_room = context.server_data.get_room_data_container().get(1);

		BOOST_CHECK(!exception);
		BOOST_CHECK((updated_room.setting_flags & pgl::room_setting_flag::open_room) == pgl::room_setting_flag::none);
		BOOST_CHECK_EQUAL(updated_room.current_player_count, 1);
	}

	BOOST_AUTO_TEST_CASE(test_update_room_status_notice_opens_room_without_reply) {
		protocol_context context;
		mark_authenticated(context);
		auto room = make_room(1, {u8"host", 1}, pgl::room_setting_flag::public_room, {}, 4, 1);
		make_room_hosted_by_client(context, room);
		const pgl::update_room_status_notice_message request{
			1,
			pgl::update_room_status_notice_message::status::open,
			true,
			2
		};
		protocol_handler_run handler(context, pgl::message_type::update_room_status);

		write_packed(context.client_socket, pgl::request_message_header{pgl::message_type::update_room_status}, request);
		const auto exception = handler.wait();
		const auto updated_room = context.server_data.get_room_data_container().get(1);

		BOOST_CHECK(!exception);
		BOOST_CHECK((updated_room.setting_flags & pgl::room_setting_flag::open_room) == pgl::room_setting_flag::open_room);
		BOOST_CHECK_EQUAL(updated_room.current_player_count, 2);
		expect_no_more_reply_data(context.client_socket);
	}

	BOOST_AUTO_TEST_CASE(test_update_room_status_notice_removes_room_without_reply) {
		protocol_context context;
		mark_authenticated(context);
		auto room = make_room(1, {u8"host", 1});
		make_room_hosted_by_client(context, room);
		context.session_data.set_hosting_room_id(1);
		const pgl::update_room_status_notice_message request{
			1,
			pgl::update_room_status_notice_message::status::remove,
			false,
			0
		};
		protocol_handler_run handler(context, pgl::message_type::update_room_status);

		write_packed(context.client_socket, pgl::request_message_header{pgl::message_type::update_room_status}, request);
		const auto exception = handler.wait();

		BOOST_CHECK(!exception);
		BOOST_CHECK(!context.server_data.get_room_data_container().contains(1));
		BOOST_CHECK(!context.session_data.is_hosting_room());
		expect_no_more_reply_data(context.client_socket);
	}

	BOOST_AUTO_TEST_CASE(test_update_room_status_notice_ignores_missing_room_without_reply) {
		protocol_context context;
		mark_authenticated(context);
		const pgl::update_room_status_notice_message request{
			404,
			pgl::update_room_status_notice_message::status::close,
			false,
			0
		};
		protocol_handler_run handler(context, pgl::message_type::update_room_status);

		write_packed(context.client_socket, pgl::request_message_header{pgl::message_type::update_room_status}, request);
		const auto exception = handler.wait();

		BOOST_CHECK(!exception);
		expect_no_more_reply_data(context.client_socket);
	}

	BOOST_AUTO_TEST_CASE(test_update_room_status_notice_ignores_host_mismatch_without_reply) {
		protocol_context context;
		mark_authenticated(context);
		context.server_data.get_room_data_container().add_or_update(make_room(1, {u8"host", 1}));
		const pgl::update_room_status_notice_message request{
			1,
			pgl::update_room_status_notice_message::status::close,
			false,
			0
		};
		protocol_handler_run handler(context, pgl::message_type::update_room_status);

		write_packed(context.client_socket, pgl::request_message_header{pgl::message_type::update_room_status}, request);
		const auto exception = handler.wait();
		const auto room = context.server_data.get_room_data_container().get(1);

		BOOST_CHECK(!exception);
		BOOST_CHECK((room.setting_flags & pgl::room_setting_flag::open_room) == pgl::room_setting_flag::open_room);
		expect_no_more_reply_data(context.client_socket);
	}

	BOOST_AUTO_TEST_CASE(test_update_room_status_notice_ignores_invalid_player_count_without_reply) {
		protocol_context context;
		mark_authenticated(context);
		auto room = make_room(1, {u8"host", 1}, public_open_room, {}, 2, 1);
		make_room_hosted_by_client(context, room);
		const pgl::update_room_status_notice_message request{
			1,
			pgl::update_room_status_notice_message::status::close,
			true,
			3
		};
		protocol_handler_run handler(context, pgl::message_type::update_room_status);

		write_packed(context.client_socket, pgl::request_message_header{pgl::message_type::update_room_status}, request);
		const auto exception = handler.wait();
		const auto updated_room = context.server_data.get_room_data_container().get(1);

		BOOST_CHECK(!exception);
		BOOST_CHECK((updated_room.setting_flags & pgl::room_setting_flag::open_room) == pgl::room_setting_flag::open_room);
		BOOST_CHECK_EQUAL(updated_room.current_player_count, 1);
		expect_no_more_reply_data(context.client_socket);
	}

	BOOST_AUTO_TEST_CASE(test_update_room_status_notice_ignores_invalid_status_without_reply) {
		protocol_context context;
		mark_authenticated(context);
		auto room = make_room(1, {u8"host", 1});
		make_room_hosted_by_client(context, room);
		const auto invalid_status = static_cast<decltype(pgl::update_room_status_notice_message::status)>(255);
		const pgl::update_room_status_notice_message request{
			1,
			invalid_status,
			false,
			0
		};
		protocol_handler_run handler(context, pgl::message_type::update_room_status);

		write_packed(context.client_socket, pgl::request_message_header{pgl::message_type::update_room_status}, request);
		const auto exception = handler.wait();
		const auto updated_room = context.server_data.get_room_data_container().get(1);

		BOOST_CHECK(!exception);
		BOOST_CHECK((updated_room.setting_flags & pgl::room_setting_flag::open_room) == pgl::room_setting_flag::open_room);
		expect_no_more_reply_data(context.client_socket);
	}

BOOST_AUTO_TEST_SUITE_END()
