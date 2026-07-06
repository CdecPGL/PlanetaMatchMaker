#include <boost/test/unit_test.hpp>

#include <boost/asio.hpp>

#include "../../PlanetaMatchMakerServer/source/network/endpoint.hpp"
#include "../../PlanetaMatchMakerServer/source/network/network_layer.hpp"
#include "../../PlanetaMatchMakerServer/source/network/transport_layer.hpp"

namespace {
	using boost::asio::ip::address_v4;
	using boost::asio::ip::address_v6;
	using boost::asio::ip::tcp;

	pgl::endpoint make_endpoint_from_address(const boost::asio::ip::address& address,
		const pgl::port_number_type port_number) {
		return pgl::endpoint::make_from_boost_endpoint(tcp::endpoint(address, port_number));
	}
}

BOOST_AUTO_TEST_SUITE(network_test)
	BOOST_AUTO_TEST_CASE(test_string_to_ip_version_accepts_supported_values) {
		BOOST_CHECK(pgl::string_to_ip_version("v4") == pgl::ip_version::v4);
		BOOST_CHECK(pgl::string_to_ip_version("v6") == pgl::ip_version::v6);
	}

	BOOST_AUTO_TEST_CASE(test_string_to_ip_version_rejects_unknown_value) {
		BOOST_CHECK_THROW(pgl::string_to_ip_version("v5"), std::out_of_range);
	}

	BOOST_AUTO_TEST_CASE(test_get_tcp_returns_protocol_for_ip_version) {
		BOOST_CHECK_EQUAL(pgl::get_tcp(pgl::ip_version::v4).family(), tcp::v4().family());
		BOOST_CHECK_EQUAL(pgl::get_tcp(pgl::ip_version::v6).family(), tcp::v6().family());
	}

	BOOST_AUTO_TEST_CASE(test_get_tcp_rejects_invalid_ip_version) {
		BOOST_CHECK_THROW(pgl::get_tcp(static_cast<pgl::ip_version>(255)), std::runtime_error);
	}

	BOOST_AUTO_TEST_CASE(test_is_port_number_valid_accepts_dynamic_private_port_range) {
		BOOST_CHECK(!pgl::is_port_number_valid(0));
		BOOST_CHECK(!pgl::is_port_number_valid(49151));
		BOOST_CHECK(pgl::is_port_number_valid(49152));
		BOOST_CHECK(pgl::is_port_number_valid(65535));
	}

	BOOST_AUTO_TEST_CASE(test_endpoint_make_from_boost_endpoint_converts_ipv4_to_mapped_ipv6_storage) {
		const auto endpoint = make_endpoint_from_address(address_v4({192, 0, 2, 1}), 57000);

		BOOST_CHECK(endpoint.ip_version() == pgl::ip_version::v4);
		BOOST_CHECK_EQUAL(endpoint.port_number, 57000);
		BOOST_CHECK_EQUAL(endpoint.ip_address[10], 0xff);
		BOOST_CHECK_EQUAL(endpoint.ip_address[11], 0xff);
		BOOST_CHECK_EQUAL(endpoint.ip_address[12], 192);
		BOOST_CHECK_EQUAL(endpoint.ip_address[13], 0);
		BOOST_CHECK_EQUAL(endpoint.ip_address[14], 2);
		BOOST_CHECK_EQUAL(endpoint.ip_address[15], 1);
	}

	BOOST_AUTO_TEST_CASE(test_endpoint_to_boost_endpoint_round_trips_ipv4_address_and_port) {
		const auto endpoint = make_endpoint_from_address(address_v4({203, 0, 113, 5}), 60000);

		const auto boost_endpoint = endpoint.to_boost_endpoint();

		BOOST_CHECK(boost_endpoint.address().is_v4());
		BOOST_CHECK_EQUAL(boost_endpoint.address().to_v4().to_string(), "203.0.113.5");
		BOOST_CHECK_EQUAL(boost_endpoint.port(), 60000);
	}

	BOOST_AUTO_TEST_CASE(test_endpoint_to_boost_endpoint_round_trips_ipv6_address_and_port) {
		const auto address = boost::asio::ip::make_address_v6("2001:db8::1");
		const auto endpoint = make_endpoint_from_address(address, 61000);

		const auto boost_endpoint = endpoint.to_boost_endpoint();

		BOOST_CHECK(endpoint.ip_version() == pgl::ip_version::v6);
		BOOST_CHECK(boost_endpoint.address().is_v6());
		BOOST_CHECK_EQUAL(boost_endpoint.address().to_v6().to_string(), "2001:db8::1");
		BOOST_CHECK_EQUAL(boost_endpoint.port(), 61000);
	}

	BOOST_AUTO_TEST_CASE(test_endpoint_equality_and_hash_use_address_and_port) {
		const auto endpoint1 = make_endpoint_from_address(address_v4({192, 0, 2, 10}), 57000);
		const auto endpoint2 = make_endpoint_from_address(address_v4({192, 0, 2, 10}), 57000);
		const auto endpoint3 = make_endpoint_from_address(address_v4({192, 0, 2, 10}), 57001);

		BOOST_CHECK(endpoint1 == endpoint2);
		BOOST_CHECK(endpoint1 != endpoint3);
		BOOST_CHECK_EQUAL(std::hash<pgl::endpoint>{}(endpoint1), std::hash<pgl::endpoint>{}(endpoint2));
	}

BOOST_AUTO_TEST_SUITE_END()
