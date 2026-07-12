#include <boost/test/unit_test.hpp>

#include "protocol_test_support.hpp"

namespace {
	using namespace pgl::test;
}

BOOST_AUTO_TEST_SUITE(list_room_protocol_test)
	BOOST_AUTO_TEST_CASE(test_list_room_request_replies_matching_room_page) {
		protocol_context context;
		mark_authenticated(context);
		context.server_data.get_room_data_container().add_or_update(make_room(1, {u8"alice", 1}));
		context.server_data.get_room_data_container().add_or_update(make_room(2, {u8"bob", 1}));
		const pgl::list_room_request_message request{
			0,
			1,
			pgl::room_data_sort_kind::name_ascending,
			pgl::room_search_target_flag::public_room | pgl::room_search_target_flag::open_room,
			{}
		};
		protocol_handler_run handler(context, pgl::message_type::list_room);

		write_packed(context.client_socket, pgl::request_message_header{pgl::message_type::list_room}, request);
		const auto reply_header = read_packed<pgl::reply_message_header>(context.client_socket);
		const auto reply = read_packed<pgl::list_room_reply_message>(context.client_socket);
		const auto exception = handler.wait();

		BOOST_CHECK(!exception);
		BOOST_CHECK(reply_header.error_code == pgl::message_error_code::ok);
		BOOST_CHECK_EQUAL(reply.total_room_count, 2);
		BOOST_CHECK_EQUAL(reply.matched_room_count, 2);
		BOOST_CHECK_EQUAL(reply.reply_room_count, 1);
		BOOST_CHECK_EQUAL(reply.room_info_list[0].room_id, pgl::room_id_t{1});
		BOOST_CHECK(reply.room_info_list[0].host_player_full_name == (pgl::player_full_name{u8"alice", 1}));
	}

	BOOST_AUTO_TEST_CASE(test_list_room_request_supports_all_sort_kinds) {
		const auto first_room_id_for = [](const pgl::room_data_sort_kind sort_kind) {
			protocol_context context;
			mark_authenticated(context);
			context.server_data.get_room_data_container().add_or_update(make_room(1, {u8"bob", 2},
				public_open_room, {}, 4, 1));
			context.server_data.get_room_data_container().add_or_update(make_room(2, {u8"alice", 1},
				public_open_room, {}, 4, 1));
			auto late_room = make_room(3, {u8"bob", 1}, public_open_room, {}, 4, 1);
			late_room.create_datetime = pgl::datetime(2024, 1, 31);
			context.server_data.get_room_data_container().add_or_update(late_room);
			const pgl::list_room_request_message request{
				0,
				1,
				sort_kind,
				pgl::room_search_target_flag::public_room | pgl::room_search_target_flag::open_room,
				{}
			};
			protocol_handler_run handler(context, pgl::message_type::list_room);

			write_packed(context.client_socket, pgl::request_message_header{pgl::message_type::list_room}, request);
			const auto reply_header = read_packed<pgl::reply_message_header>(context.client_socket);
			const auto reply = read_packed<pgl::list_room_reply_message>(context.client_socket);
			const auto exception = handler.wait();

			BOOST_CHECK(!exception);
			BOOST_CHECK(reply_header.error_code == pgl::message_error_code::ok);
			return reply.room_info_list[0].room_id;
		};

		BOOST_CHECK_EQUAL(first_room_id_for(pgl::room_data_sort_kind::name_ascending), pgl::room_id_t{2});
		BOOST_CHECK_EQUAL(first_room_id_for(pgl::room_data_sort_kind::name_descending), pgl::room_id_t{1});
		BOOST_CHECK_EQUAL(first_room_id_for(pgl::room_data_sort_kind::create_datetime_ascending), pgl::room_id_t{1});
		BOOST_CHECK_EQUAL(first_room_id_for(pgl::room_data_sort_kind::create_datetime_descending), pgl::room_id_t{3});
	}

	BOOST_AUTO_TEST_CASE(test_list_room_request_replies_empty_result_body) {
		protocol_context context;
		mark_authenticated(context);
		const pgl::list_room_request_message request{
			0,
			10,
			pgl::room_data_sort_kind::name_ascending,
			pgl::room_search_target_flag::public_room | pgl::room_search_target_flag::open_room,
			{}
		};
		protocol_handler_run handler(context, pgl::message_type::list_room);

		write_packed(context.client_socket, pgl::request_message_header{pgl::message_type::list_room}, request);
		const auto reply_header = read_packed<pgl::reply_message_header>(context.client_socket);
		const auto reply = read_packed<pgl::list_room_reply_message>(context.client_socket);
		const auto exception = handler.wait();

		BOOST_CHECK(!exception);
		BOOST_CHECK(reply_header.error_code == pgl::message_error_code::ok);
		BOOST_CHECK_EQUAL(reply.total_room_count, 0);
		BOOST_CHECK_EQUAL(reply.matched_room_count, 0);
		BOOST_CHECK_EQUAL(reply.reply_room_count, 0);
		expect_no_more_reply_data(context.client_socket);
	}

	BOOST_AUTO_TEST_CASE(test_list_room_request_replies_zero_rooms_when_start_index_is_out_of_range) {
		protocol_context context;
		mark_authenticated(context);
		context.server_data.get_room_data_container().add_or_update(make_room(1, {u8"alice", 1}));
		context.server_data.get_room_data_container().add_or_update(make_room(2, {u8"bob", 1}));
		const pgl::list_room_request_message request{
			3,
			10,
			pgl::room_data_sort_kind::name_ascending,
			pgl::room_search_target_flag::public_room | pgl::room_search_target_flag::open_room,
			{}
		};
		protocol_handler_run handler(context, pgl::message_type::list_room);

		write_packed(context.client_socket, pgl::request_message_header{pgl::message_type::list_room}, request);
		const auto reply_header = read_packed<pgl::reply_message_header>(context.client_socket);
		const auto reply = read_packed<pgl::list_room_reply_message>(context.client_socket);
		const auto exception = handler.wait();

		BOOST_CHECK(!exception);
		BOOST_CHECK(reply_header.error_code == pgl::message_error_code::ok);
		BOOST_CHECK_EQUAL(reply.total_room_count, 2);
		BOOST_CHECK_EQUAL(reply.matched_room_count, 2);
		BOOST_CHECK_EQUAL(reply.reply_room_count, 0);
		expect_no_more_reply_data(context.client_socket);
	}

	BOOST_AUTO_TEST_CASE(test_list_room_request_splits_replies_when_result_exceeds_one_message_capacity) {
		protocol_context context;
		mark_authenticated(context);
		for (auto i = pgl::room_id_t{1}; i <= 7; ++i) {
			context.server_data.get_room_data_container().add_or_update(
				make_room(i, {u8"host", static_cast<pgl::player_tag_t>(i)}));
		}
		const pgl::list_room_request_message request{
			0,
			7,
			pgl::room_data_sort_kind::create_datetime_ascending,
			pgl::room_search_target_flag::public_room | pgl::room_search_target_flag::open_room,
			{}
		};
		protocol_handler_run handler(context, pgl::message_type::list_room);

		write_packed(context.client_socket, pgl::request_message_header{pgl::message_type::list_room}, request);
		const auto first_header = read_packed<pgl::reply_message_header>(context.client_socket);
		const auto first_reply = read_packed<pgl::list_room_reply_message>(context.client_socket);
		const auto second_header = read_packed<pgl::reply_message_header>(context.client_socket);
		const auto second_reply = read_packed<pgl::list_room_reply_message>(context.client_socket);
		const auto exception = handler.wait();

		BOOST_CHECK(!exception);
		BOOST_CHECK(first_header.error_code == pgl::message_error_code::ok);
		BOOST_CHECK(second_header.error_code == pgl::message_error_code::ok);
		BOOST_CHECK_EQUAL(first_reply.reply_room_count, 7);
		BOOST_CHECK_EQUAL(first_reply.room_info_list[0].room_id, pgl::room_id_t{1});
		BOOST_CHECK_EQUAL(first_reply.room_info_list[5].room_id, pgl::room_id_t{6});
		BOOST_CHECK_EQUAL(second_reply.reply_room_count, 7);
		BOOST_CHECK_EQUAL(second_reply.room_info_list[0].room_id, pgl::room_id_t{7});
		BOOST_CHECK_EQUAL(second_reply.room_info_list[1].room_id, pgl::room_id_t{0});
		expect_no_more_reply_data(context.client_socket);
	}

	BOOST_AUTO_TEST_CASE(test_list_room_request_filters_by_room_status_name_and_tag) {
		protocol_context context;
		mark_authenticated(context);
		context.server_data.get_room_data_container().add_or_update(make_room(1, {u8"alice", 7},
			pgl::room_setting_flag::none));
		context.server_data.get_room_data_container().add_or_update(make_room(2, {u8"alina", 8},
			pgl::room_setting_flag::none));
		context.server_data.get_room_data_container().add_or_update(make_room(3, {u8"alicia", 7},
			pgl::room_setting_flag::open_room));
		context.server_data.get_room_data_container().add_or_update(make_room(4, {u8"alice-public", 7},
			public_open_room));
		const pgl::list_room_request_message request{
			0,
			10,
			pgl::room_data_sort_kind::name_ascending,
			pgl::room_search_target_flag::private_room | pgl::room_search_target_flag::closed_room,
			{u8"ali", 7}
		};
		protocol_handler_run handler(context, pgl::message_type::list_room);

		write_packed(context.client_socket, pgl::request_message_header{pgl::message_type::list_room}, request);
		const auto reply_header = read_packed<pgl::reply_message_header>(context.client_socket);
		const auto reply = read_packed<pgl::list_room_reply_message>(context.client_socket);
		const auto exception = handler.wait();

		BOOST_CHECK(!exception);
		BOOST_CHECK(reply_header.error_code == pgl::message_error_code::ok);
		BOOST_CHECK_EQUAL(reply.total_room_count, 4);
		BOOST_CHECK_EQUAL(reply.matched_room_count, 1);
		BOOST_CHECK_EQUAL(reply.reply_room_count, 1);
		BOOST_CHECK_EQUAL(reply.room_info_list[0].room_id, pgl::room_id_t{1});
	}

	BOOST_AUTO_TEST_CASE(test_list_room_request_replies_parameter_error_for_invalid_sort_kind) {
		protocol_context context;
		mark_authenticated(context);
		const pgl::list_room_request_message request{
			0,
			10,
			static_cast<pgl::room_data_sort_kind>(255),
			pgl::room_search_target_flag::public_room | pgl::room_search_target_flag::open_room,
			{}
		};
		protocol_handler_run handler(context, pgl::message_type::list_room);

		write_packed(context.client_socket, pgl::request_message_header{pgl::message_type::list_room}, request);
		const auto reply_header = read_packed<pgl::reply_message_header>(context.client_socket);
		const auto exception = handler.wait();

		BOOST_CHECK(!exception);
		BOOST_CHECK(reply_header.error_code == pgl::message_error_code::request_parameter_wrong);
		expect_no_more_reply_data(context.client_socket);
	}

BOOST_AUTO_TEST_SUITE_END()
