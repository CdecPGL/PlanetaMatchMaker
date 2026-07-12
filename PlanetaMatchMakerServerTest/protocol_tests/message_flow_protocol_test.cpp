#include <boost/test/unit_test.hpp>

#include "protocol_test_support.hpp"

namespace {
	using namespace pgl::test;
}

BOOST_AUTO_TEST_SUITE(message_flow_protocol_test)
	BOOST_AUTO_TEST_CASE(test_keep_alive_notice_completes_without_reply) {
		protocol_context context;
		mark_authenticated(context);
		const pgl::keep_alive_notice_message request{};
		protocol_handler_run handler(context, pgl::message_type::keep_alive);

		write_packed(context.client_socket, pgl::request_message_header{pgl::message_type::keep_alive}, request);
		const auto exception = handler.wait();

		BOOST_CHECK(!exception);
	}

	BOOST_AUTO_TEST_CASE(test_protocol_flow_disconnects_without_reply_for_message_before_authentication) {
		protocol_context context;
		const pgl::keep_alive_notice_message request{};
		protocol_handler_run handler(context, pgl::message_type::keep_alive);

		write_packed(context.client_socket, pgl::request_message_header{pgl::message_type::keep_alive}, request);
		const auto exception = handler.wait();

		BOOST_CHECK(is_intended_disconnect(exception));
		expect_no_more_reply_data(context.client_socket);
	}

	BOOST_AUTO_TEST_CASE(test_protocol_flow_disconnects_without_reply_for_invalid_message_type) {
		protocol_context context;
		protocol_handler_run handler(context);

		write_packed(context.client_socket,
			pgl::request_message_header{static_cast<pgl::message_type>(200)});
		const auto exception = handler.wait();

		BOOST_CHECK(is_intended_disconnect(exception));
		expect_no_more_reply_data(context.client_socket);
	}

	BOOST_AUTO_TEST_CASE(test_protocol_flow_disconnects_without_reply_when_first_message_is_not_authentication) {
		protocol_context context;
		protocol_handler_run handler(context, pgl::message_type::authentication);

		write_packed(context.client_socket, pgl::request_message_header{pgl::message_type::create_room});
		const auto exception = handler.wait();

		BOOST_CHECK(is_intended_disconnect(exception));
		expect_no_more_reply_data(context.client_socket);
	}

	BOOST_AUTO_TEST_CASE(test_protocol_flow_disconnects_without_reply_for_unimplemented_random_match) {
		protocol_context context;
		protocol_handler_run handler(context);

		write_packed(context.client_socket, pgl::request_message_header{pgl::message_type::random_match});
		const auto exception = handler.wait();

		BOOST_CHECK(is_intended_disconnect(exception));
		expect_no_more_reply_data(context.client_socket);
	}

BOOST_AUTO_TEST_SUITE_END()
