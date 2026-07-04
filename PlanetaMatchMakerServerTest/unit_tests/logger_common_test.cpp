#include <boost/test/unit_test.hpp>

#include <sstream>
#include <string>
#include <thread>

#include "../../PlanetaMatchMakerServer/source/logger/logger_common.hpp"

BOOST_AUTO_TEST_SUITE(logger_common_test)
	BOOST_AUTO_TEST_CASE(test_output_formatted_log_includes_thread_id) {
		std::ostringstream out_stream;
		pgl::output_formatted_log(out_stream, pgl::log_level::info, "", "test message");

		std::ostringstream thread_id_stream;
		thread_id_stream << std::this_thread::get_id();

		const auto output = out_stream.str();
		BOOST_CHECK_NE(output.find("[thread:" + thread_id_stream.str() + "]"), std::string::npos);
		BOOST_CHECK_NE(output.find("info: test message"), std::string::npos);
	}

BOOST_AUTO_TEST_SUITE_END()
