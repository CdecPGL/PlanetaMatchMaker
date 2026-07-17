#include <boost/test/unit_test.hpp>

#include <sstream>

#include "client/client_error_code.hpp"
#include "client/client_errors.hpp"
#include "message/message_error_code.hpp"
#include "server/server_errors.hpp"

BOOST_AUTO_TEST_SUITE(errors_test)
	BOOST_AUTO_TEST_CASE(test_client_error_stores_code_disconnect_flag_and_extra_message) {
		const pgl::client_error error(pgl::client_error_code::room_full, true, "room is full");

		BOOST_CHECK(error.error_code() == pgl::client_error_code::room_full);
		BOOST_CHECK(error.is_disconnect_required());
		BOOST_CHECK_EQUAL(error.extra_message(), "room is full");
		BOOST_CHECK_EQUAL(error.message(), "Client error \"room_full\". (room is full)");
	}

	BOOST_AUTO_TEST_CASE(test_client_error_message_without_extra_message) {
		const pgl::client_error error(pgl::client_error_code::operation_invalid, false);

		BOOST_CHECK(!error.is_disconnect_required());
		BOOST_CHECK_EQUAL(error.extra_message(), "");
		BOOST_CHECK_EQUAL(error.message(), "Client error \"operation_invalid\".");
	}

	BOOST_AUTO_TEST_CASE(test_client_error_stream_operator_outputs_message) {
		const pgl::client_error error(pgl::client_error_code::room_not_found, false, "missing");
		std::ostringstream stream;

		stream << error;

		BOOST_CHECK_EQUAL(stream.str(), error.message());
	}

	BOOST_AUTO_TEST_CASE(test_client_error_code_stream_operator_outputs_enum_name) {
		std::ostringstream stream;

		stream << pgl::client_error_code::room_password_wrong;

		BOOST_CHECK_EQUAL(stream.str(), "room_password_wrong");
	}

	BOOST_AUTO_TEST_CASE(test_message_error_code_is_converted_from_client_error_code) {
		BOOST_CHECK(pgl::get_message_error_code_from_client_error_code(
			pgl::client_error_code::operation_invalid) == pgl::message_error_code::operation_invalid);
		BOOST_CHECK(pgl::get_message_error_code_from_client_error_code(
			pgl::client_error_code::request_parameter_wrong) == pgl::message_error_code::request_parameter_wrong);
		BOOST_CHECK(pgl::get_message_error_code_from_client_error_code(
			pgl::client_error_code::room_not_found) == pgl::message_error_code::room_not_found);
		BOOST_CHECK(pgl::get_message_error_code_from_client_error_code(
			pgl::client_error_code::room_password_wrong) == pgl::message_error_code::room_password_wrong);
		BOOST_CHECK(pgl::get_message_error_code_from_client_error_code(
			pgl::client_error_code::room_full) == pgl::message_error_code::room_full);
		BOOST_CHECK(pgl::get_message_error_code_from_client_error_code(
			pgl::client_error_code::room_permission_denied) == pgl::message_error_code::room_permission_denied);
		BOOST_CHECK(pgl::get_message_error_code_from_client_error_code(
			pgl::client_error_code::room_count_exceeds_limit) == pgl::message_error_code::room_count_exceeds_limit);
		BOOST_CHECK(pgl::get_message_error_code_from_client_error_code(
			pgl::client_error_code::room_connection_establish_mode_mismatch) ==
			pgl::message_error_code::room_connection_establish_mode_mismatch);
		BOOST_CHECK(pgl::get_message_error_code_from_client_error_code(
			pgl::client_error_code::client_already_hosting_room) ==
			pgl::message_error_code::client_already_hosting_room);
	}

	BOOST_AUTO_TEST_CASE(test_message_error_code_wire_values) {
		BOOST_CHECK_EQUAL(static_cast<unsigned int>(pgl::message_error_code::ok), 0u);
		BOOST_CHECK_EQUAL(static_cast<unsigned int>(pgl::message_error_code::server_error), 1u);
		BOOST_CHECK_EQUAL(static_cast<unsigned int>(pgl::message_error_code::operation_invalid), 2u);
		BOOST_CHECK_EQUAL(static_cast<unsigned int>(pgl::message_error_code::request_parameter_wrong), 3u);
		BOOST_CHECK_EQUAL(static_cast<unsigned int>(pgl::message_error_code::room_not_found), 4u);
		BOOST_CHECK_EQUAL(static_cast<unsigned int>(pgl::message_error_code::room_password_wrong), 5u);
		BOOST_CHECK_EQUAL(static_cast<unsigned int>(pgl::message_error_code::room_full), 6u);
		BOOST_CHECK_EQUAL(static_cast<unsigned int>(pgl::message_error_code::room_permission_denied), 7u);
		BOOST_CHECK_EQUAL(static_cast<unsigned int>(pgl::message_error_code::room_count_exceeds_limit), 8u);
		BOOST_CHECK_EQUAL(static_cast<unsigned int>(
			pgl::message_error_code::room_connection_establish_mode_mismatch), 9u);
		BOOST_CHECK_EQUAL(static_cast<unsigned int>(pgl::message_error_code::client_already_hosting_room), 10u);
	}

	BOOST_AUTO_TEST_CASE(test_server_error_stores_disconnect_flag_and_extra_message) {
		const pgl::server_error error(true, "database unavailable");

		BOOST_CHECK(error.is_disconnect_required());
		BOOST_CHECK_EQUAL(error.extra_message(), "database unavailable");
		BOOST_CHECK_EQUAL(error.message(), "Server error. (database unavailable)");
	}

	BOOST_AUTO_TEST_CASE(test_server_error_message_without_extra_message) {
		const pgl::server_error error(false);

		BOOST_CHECK(!error.is_disconnect_required());
		BOOST_CHECK_EQUAL(error.extra_message(), "");
		BOOST_CHECK_EQUAL(error.message(), "Server error.");
	}

	BOOST_AUTO_TEST_CASE(test_server_error_stream_operator_outputs_message) {
		const pgl::server_error error(false, "failed");
		std::ostringstream stream;

		stream << error;

		BOOST_CHECK_EQUAL(stream.str(), error.message());
	}

	BOOST_AUTO_TEST_CASE(test_server_session_intended_disconnect_error_message) {
		const pgl::server_session_intended_disconnect_error error("finished");

		BOOST_CHECK_EQUAL(error.extra_message(), "finished");
		BOOST_CHECK_EQUAL(error.message(), "Server error. (finished)");
	}

	BOOST_AUTO_TEST_CASE(test_server_session_intended_disconnect_error_message_without_extra_message) {
		const pgl::server_session_intended_disconnect_error error;

		BOOST_CHECK_EQUAL(error.extra_message(), "");
		BOOST_CHECK_EQUAL(error.message(), "Server error.");
	}

	BOOST_AUTO_TEST_CASE(test_server_session_intended_disconnect_error_stream_operator_outputs_message) {
		const pgl::server_session_intended_disconnect_error error("finished");
		std::ostringstream stream;

		stream << error;

		BOOST_CHECK_EQUAL(stream.str(), error.message());
	}

	BOOST_AUTO_TEST_CASE(test_server_session_error_message) {
		const pgl::server_session_error error("receive failed");

		BOOST_CHECK_EQUAL(error.extra_message(), "receive failed");
		BOOST_CHECK_EQUAL(error.message(), "Session error. receive failed");
	}

	BOOST_AUTO_TEST_CASE(test_server_session_error_stream_operator_outputs_message) {
		const pgl::server_session_error error("send failed");
		std::ostringstream stream;

		stream << error;

		BOOST_CHECK_EQUAL(stream.str(), error.message());
	}

BOOST_AUTO_TEST_SUITE_END()
