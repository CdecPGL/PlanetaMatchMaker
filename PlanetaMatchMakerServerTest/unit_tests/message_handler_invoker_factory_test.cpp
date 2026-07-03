#include <boost/test/unit_test.hpp>

#include "../../PlanetaMatchMakerServer/source/message/message_handler_invoker.hpp"
#include "../../PlanetaMatchMakerServer/source/message/message_handler_invoker_factory.hpp"

BOOST_AUTO_TEST_SUITE(message_handler_invoker_factory_test)
	BOOST_AUTO_TEST_CASE(test_make_shared_standard_returns_invoker) {
		const auto invoker = pgl::message_handler_invoker_factory::make_shared_standard();

		BOOST_CHECK(invoker != nullptr);
	}

	BOOST_AUTO_TEST_CASE(test_make_unique_standard_returns_invoker) {
		const auto invoker = pgl::message_handler_invoker_factory::make_unique_standard();

		BOOST_CHECK(invoker != nullptr);
	}

BOOST_AUTO_TEST_SUITE_END()
