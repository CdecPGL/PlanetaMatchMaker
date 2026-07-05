#include <boost/test/unit_test.hpp>

#include <boost/asio.hpp>

#include <sstream>

#include "../../PlanetaMatchMakerServer/source/room/room_data.hpp"
#include "../../PlanetaMatchMakerServer/source/utilities/asio_stream_compatibility.hpp"
#include "../../PlanetaMatchMakerServer/source/utilities/file_utilities.hpp"
#include "../../PlanetaMatchMakerServer/source/data/random_id_generator.hpp"

BOOST_AUTO_TEST_SUITE(utilities_test)
	BOOST_AUTO_TEST_CASE(test_room_setting_flag_supports_bitwise_operators) {
		auto flags = pgl::room_setting_flag::public_room | pgl::room_setting_flag::open_room;

		BOOST_CHECK((flags & pgl::room_setting_flag::public_room) == pgl::room_setting_flag::public_room);
		BOOST_CHECK((flags & pgl::room_setting_flag::open_room) == pgl::room_setting_flag::open_room);

		flags &= pgl::room_setting_flag::public_room;

		BOOST_CHECK(flags == pgl::room_setting_flag::public_room);

		flags ^= pgl::room_setting_flag::open_room;

		BOOST_CHECK((flags & pgl::room_setting_flag::open_room) == pgl::room_setting_flag::open_room);
	}

	BOOST_AUTO_TEST_CASE(test_room_search_target_flag_supports_bitwise_operators) {
		auto flags = pgl::room_search_target_flag::public_room | pgl::room_search_target_flag::closed_room;

		BOOST_CHECK((flags & pgl::room_search_target_flag::public_room) ==
			pgl::room_search_target_flag::public_room);
		BOOST_CHECK((flags & pgl::room_search_target_flag::closed_room) ==
			pgl::room_search_target_flag::closed_room);

		flags |= pgl::room_search_target_flag::open_room;

		BOOST_CHECK((flags & pgl::room_search_target_flag::open_room) ==
			pgl::room_search_target_flag::open_room);
	}

	BOOST_AUTO_TEST_CASE(test_generate_random_id_supports_regular_and_eight_bit_integral_types) {
		[[maybe_unused]] const auto id32 = pgl::generate_random_id<uint32_t>();
		[[maybe_unused]] const auto id8 = pgl::generate_random_id<uint8_t>();

		BOOST_CHECK(true);
	}

	BOOST_AUTO_TEST_CASE(test_get_home_directory_returns_non_empty_path) {
		BOOST_CHECK(!pgl::get_home_directory().empty());
	}

	BOOST_AUTO_TEST_CASE(test_application_setting_and_log_directories_use_expected_names) {
		BOOST_CHECK_EQUAL(pgl::get_application_setting_directory().filename().string(), "pmms");
		BOOST_CHECK_EQUAL(pgl::get_application_log_directory().filename().string(), "log");
	}

	BOOST_AUTO_TEST_CASE(test_tcp_endpoint_stream_operator_outputs_address_and_port) {
		const boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::address_v4({127, 0, 0, 1}), 57000);
		std::ostringstream stream;

		boost::asio::operator<<(stream, endpoint);

		BOOST_CHECK_EQUAL(stream.str(), "endpoint(127.0.0.1:57000)");
	}

	BOOST_AUTO_TEST_CASE(test_error_code_stream_operator_outputs_message) {
		const boost::system::error_code error_code = boost::asio::error::operation_aborted;
		std::ostringstream stream;

		stream << error_code;

		BOOST_CHECK_EQUAL(stream.str(), error_code.message());
	}

	BOOST_AUTO_TEST_CASE(test_system_error_stream_operator_outputs_error_code_message) {
		const boost::system::system_error error(boost::asio::error::operation_aborted);
		std::ostringstream stream;

		stream << error;

		BOOST_CHECK_EQUAL(stream.str(), error.code().message());
	}

BOOST_AUTO_TEST_SUITE_END()
