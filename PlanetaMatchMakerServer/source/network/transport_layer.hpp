#pragma once

#include <cstdint>

#include <boost/asio.hpp>

#include "network_layer.hpp"

namespace pgl {
	boost::asio::ip::tcp get_tcp(ip_version ip_version);
	using port_number_type = uint16_t;
}
