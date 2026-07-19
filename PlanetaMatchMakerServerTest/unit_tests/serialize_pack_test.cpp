#include <boost/test/unit_test.hpp>

#include <cstddef>

#include "message/messages.hpp"
#include "session/session_data.hpp"
#include "utilities/pack.hpp"

BOOST_AUTO_TEST_SUITE(serialize_pack_test)

	struct test_struct1 final {
		uint8_t value1;
		uint64_t value2;
		std::array<uint8_t, 4> value3;

		using serialize_targets = minimal_serializer::serialize_target_container<
			&test_struct1::value1,
			&test_struct1::value2,
			&test_struct1::value3
		>;
	};

	struct test_struct2 final {
		int64_t value1;
		std::array<uint16_t, 8> value2;

		using serialize_targets = minimal_serializer::serialize_target_container<
			&test_struct2::value1,
			&test_struct2::value2
		>;
	};

	BOOST_AUTO_TEST_CASE(test_packed_one_data_size) {
		using test_t = test_struct1;
		const test_t data{};
		const auto expected = minimal_serializer::serialized_size_v<test_t>;
		const auto actual = pgl::pack_data(data).size();
		BOOST_CHECK_EQUAL(expected, actual);
	}

	BOOST_AUTO_TEST_CASE(test_packed_two_data_size) {
		using test_t1 = test_struct1;
		using test_t2 = test_struct2;
		const test_t1 data1{};
		const test_t2 data2{};
		const auto expected = minimal_serializer::serialized_size_v<test_t1> + minimal_serializer::serialized_size_v<
			test_t2>;
		const auto actual = pgl::pack_data(data1, data2).size();
		BOOST_CHECK_EQUAL(expected, actual);
	}

	BOOST_AUTO_TEST_CASE(test_pack_and_unpack_one_data) {
		using test_t = uint64_t;
		const test_t expected{123};

		const auto actual_buffer = pgl::pack_data(expected);

		test_t actual;
		pgl::unpack_data(actual_buffer, actual);

		BOOST_CHECK_EQUAL(expected, actual);
	}

	BOOST_AUTO_TEST_CASE(test_pack_and_unpack_two_data) {
		using test_t1 = uint64_t;
		using test_t2 = int64_t;
		const test_t1 expected1{123};
		const test_t2 expected2{-23452123};

		const auto actual_buffer = pgl::pack_data(expected1, expected2);

		test_t1 actual1;
		test_t2 actual2;
		pgl::unpack_data(actual_buffer, actual1, actual2);

		BOOST_CHECK_EQUAL(expected1, actual1);
		BOOST_CHECK_EQUAL(expected2, actual2);
	}

	BOOST_AUTO_TEST_CASE(test_unpack_lack_of_data_exception) {
		using test_t = uint64_t;
		test_t actual;
		BOOST_CHECK_EXCEPTION(pgl::unpack_data(std::vector<uint8_t>(1), actual),
			minimal_serializer::serialization_error,
			[](auto) {return true; });
	}

	BOOST_AUTO_TEST_CASE(test_message_protocol_serialized_sizes) {
		BOOST_CHECK_EQUAL(minimal_serializer::serialized_size_v<pgl::request_message_header>, std::size_t{5});
		BOOST_CHECK_EQUAL(minimal_serializer::serialized_size_v<pgl::reply_message_header>, std::size_t{6});
		BOOST_CHECK_EQUAL(minimal_serializer::serialized_size_v<pgl::authentication_request_message>, std::size_t{75});
		BOOST_CHECK_EQUAL(minimal_serializer::serialized_size_v<pgl::message_attachment_chunk>, std::size_t{243});
		BOOST_CHECK_EQUAL(minimal_serializer::serialized_size_v<pgl::create_room_request_message>, std::size_t{148});
		BOOST_CHECK_EQUAL(minimal_serializer::serialized_size_v<pgl::join_room_reply_message>, std::size_t{146});
		BOOST_CHECK_EQUAL(minimal_serializer::serialized_size_v<pgl::list_room_reply_message>, std::size_t{216});
		BOOST_CHECK_EQUAL(minimal_serializer::serialized_size_v<pgl::request_message_header> +
			minimal_serializer::serialized_size_v<pgl::create_room_request_message>, std::size_t{153});
		BOOST_CHECK_EQUAL(minimal_serializer::serialized_size_v<pgl::reply_message_header> +
			minimal_serializer::serialized_size_v<pgl::join_room_reply_message>, std::size_t{152});
	}

	BOOST_AUTO_TEST_CASE(test_fixed_id_types_accept_128_bytes_and_reject_129_bytes) {
		BOOST_CHECK_NO_THROW(pgl::p2p_service_peer_id_t(std::u8string(128, u8'p')));
		BOOST_CHECK_THROW(pgl::p2p_service_peer_id_t(std::u8string(129, u8'p')), std::out_of_range);
		BOOST_CHECK_NO_THROW(pgl::authentication_provider_user_id_t(std::u8string(128, u8'a')));
		BOOST_CHECK_THROW(pgl::authentication_provider_user_id_t(std::u8string(129, u8'a')), std::out_of_range);
	}

	BOOST_AUTO_TEST_CASE(test_all_message_records_fit_within_256_bytes) {

		constexpr std::size_t max_record_size = 256;
		constexpr auto request_header_size = minimal_serializer::serialized_size_v<pgl::request_message_header>;
		constexpr auto reply_header_size = minimal_serializer::serialized_size_v<pgl::reply_message_header>;
		BOOST_CHECK_LE(request_header_size +
			minimal_serializer::serialized_size_v<pgl::authentication_request_message>, max_record_size);
		BOOST_CHECK_LE(reply_header_size +
			minimal_serializer::serialized_size_v<pgl::authentication_reply_message>, max_record_size);
		BOOST_CHECK_LE(request_header_size +
			minimal_serializer::serialized_size_v<pgl::create_room_request_message>, max_record_size);
		BOOST_CHECK_LE(reply_header_size +
			minimal_serializer::serialized_size_v<pgl::create_room_reply_message>, max_record_size);
		BOOST_CHECK_LE(request_header_size +
			minimal_serializer::serialized_size_v<pgl::list_room_request_message>, max_record_size);
		BOOST_CHECK_LE(reply_header_size +
			minimal_serializer::serialized_size_v<pgl::list_room_reply_message>, max_record_size);
		BOOST_CHECK_LE(request_header_size +
			minimal_serializer::serialized_size_v<pgl::join_room_request_message>, max_record_size);
		BOOST_CHECK_LE(reply_header_size +
			minimal_serializer::serialized_size_v<pgl::join_room_reply_message>, max_record_size);
		BOOST_CHECK_LE(request_header_size +
			minimal_serializer::serialized_size_v<pgl::update_room_status_notice_message>, max_record_size);
		BOOST_CHECK_LE(request_header_size +
			minimal_serializer::serialized_size_v<pgl::connection_test_request_message>, max_record_size);
		BOOST_CHECK_LE(reply_header_size +
			minimal_serializer::serialized_size_v<pgl::connection_test_reply_message>, max_record_size);
		BOOST_CHECK_LE(request_header_size +
			minimal_serializer::serialized_size_v<pgl::random_match_request_message>, max_record_size);
		BOOST_CHECK_LE(request_header_size +
			minimal_serializer::serialized_size_v<pgl::keep_alive_notice_message>, max_record_size);
		BOOST_CHECK_LE(minimal_serializer::serialized_size_v<pgl::message_attachment_chunk>, max_record_size);
	}

BOOST_AUTO_TEST_SUITE_END()
