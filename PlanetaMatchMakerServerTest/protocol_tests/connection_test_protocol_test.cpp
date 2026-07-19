#include <boost/test/unit_test.hpp>

#include "protocol_test_support.hpp"

namespace {
	using namespace pgl::test;
}

BOOST_AUTO_TEST_SUITE(connection_test_protocol_test)
	BOOST_AUTO_TEST_CASE(test_connection_test_request_replies_success_for_tcp_echo) {
		tcp_echo_server echo_server;
		protocol_context context;
		mark_authenticated(context);
		const pgl::connection_test_request_message request{
			pgl::transport_protocol::tcp,
			echo_server.port()
		};
		protocol_handler_run handler(context, pgl::message_type::connection_test);

		write_packed(context.client_socket, pgl::request_message_header{pgl::message_type::connection_test}, request);
		const auto reply_header = read_packed<pgl::reply_message_header>(context.client_socket);
		const auto reply = read_packed<pgl::connection_test_reply_message>(context.client_socket);
		const auto exception = handler.wait();

		BOOST_CHECK(!exception);
		BOOST_CHECK(reply_header.error_code == pgl::message_error_code::ok);
		BOOST_CHECK(reply.succeed);
	}

	BOOST_AUTO_TEST_CASE(test_connection_test_request_replies_parameter_error_for_invalid_protocol) {
		protocol_context context;
		mark_authenticated(context);
		const pgl::connection_test_request_message request{
			static_cast<pgl::transport_protocol>(255),
			57000
		};
		protocol_handler_run handler(context, pgl::message_type::connection_test);

		write_packed(context.client_socket, pgl::request_message_header{pgl::message_type::connection_test}, request);
		const auto reply_header = read_packed<pgl::reply_message_header>(context.client_socket);
		const auto exception = handler.wait();

		BOOST_CHECK(!exception);
		BOOST_CHECK(reply_header.message_type == pgl::message_type::connection_test);
		BOOST_CHECK(reply_header.error_code == pgl::message_error_code::request_parameter_wrong);
	}

	BOOST_AUTO_TEST_CASE(test_connection_test_request_replies_success_for_udp_echo) {
		udp_echo_server echo_server;
		protocol_context context;
		mark_authenticated(context);
		const pgl::connection_test_request_message request{
			pgl::transport_protocol::udp,
			echo_server.port()
		};
		protocol_handler_run handler(context, pgl::message_type::connection_test);

		write_packed(context.client_socket, pgl::request_message_header{pgl::message_type::connection_test}, request);
		const auto reply_header = read_packed<pgl::reply_message_header>(context.client_socket);
		const auto reply = read_packed<pgl::connection_test_reply_message>(context.client_socket);
		const auto exception = handler.wait();

		BOOST_CHECK(!exception);
		BOOST_CHECK(reply_header.error_code == pgl::message_error_code::ok);
		BOOST_CHECK(reply.succeed);
	}

	BOOST_AUTO_TEST_CASE(test_connection_test_request_replies_failure_for_unreachable_tcp_endpoint) {
		protocol_context context;
		mark_authenticated(context);
		const pgl::connection_test_request_message request{
			pgl::transport_protocol::tcp,
			find_unused_dynamic_private_tcp_port()
		};
		protocol_handler_run handler(context, pgl::message_type::connection_test);

		write_packed(context.client_socket, pgl::request_message_header{pgl::message_type::connection_test}, request);
		const auto reply_header = read_packed<pgl::reply_message_header>(context.client_socket);
		const auto reply = read_packed<pgl::connection_test_reply_message>(context.client_socket);
		const auto exception = handler.wait();

		BOOST_CHECK(!exception);
		BOOST_CHECK(reply_header.error_code == pgl::message_error_code::ok);
		BOOST_CHECK(!reply.succeed);
	}

	BOOST_AUTO_TEST_CASE(test_connection_test_request_replies_failure_for_unreachable_udp_endpoint) {
		udp_port_guard port_guard;
		protocol_context context;
		mark_authenticated(context);
		context.setting.connection_test.connection_check_udp_time_out_seconds = 1;
		context.setting.connection_test.connection_check_udp_try_count = 1;
		const pgl::connection_test_request_message request{
			pgl::transport_protocol::udp,
			port_guard.port()
		};
		protocol_handler_run handler(context, pgl::message_type::connection_test);

		write_packed(context.client_socket, pgl::request_message_header{pgl::message_type::connection_test}, request);
		const auto reply_header = read_packed<pgl::reply_message_header>(context.client_socket);
		const auto reply = read_packed<pgl::connection_test_reply_message>(context.client_socket);
		const auto exception = handler.wait();

		BOOST_CHECK(!exception);
		BOOST_CHECK(reply_header.error_code == pgl::message_error_code::ok);
		BOOST_CHECK(!reply.succeed);
	}

	BOOST_AUTO_TEST_CASE(test_connection_test_request_replies_parameter_error_for_invalid_port) {
		protocol_context context;
		mark_authenticated(context);
		const pgl::connection_test_request_message request{
			pgl::transport_protocol::tcp,
			49151
		};
		protocol_handler_run handler(context, pgl::message_type::connection_test);

		write_packed(context.client_socket, pgl::request_message_header{pgl::message_type::connection_test}, request);
		const auto reply_header = read_packed<pgl::reply_message_header>(context.client_socket);
		const auto exception = handler.wait();

		BOOST_CHECK(!exception);
		BOOST_CHECK(reply_header.error_code == pgl::message_error_code::request_parameter_wrong);
		expect_no_more_reply_data(context.client_socket);
	}
BOOST_AUTO_TEST_SUITE_END()
