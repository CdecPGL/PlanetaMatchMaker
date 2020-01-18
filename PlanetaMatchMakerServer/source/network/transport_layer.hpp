#pragma once

#include <cstdint>

#include <boost/asio.hpp>

#include "network_layer.hpp"

namespace pgl {
	enum class transport_protocol : uint8_t { tcp, udp };

	using port_number_type = uint16_t;

	boost::asio::ip::tcp get_tcp(ip_version ip_version);

	// check if the port number is valid. (dynamic/private ports are considered as valid port)
	bool is_port_number_valid(port_number_type port_number);
}
