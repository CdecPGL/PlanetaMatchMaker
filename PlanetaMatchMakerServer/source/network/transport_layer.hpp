#pragma once

#include <boost/asio.hpp>

#include "network_layer.hpp"

namespace pgl {
	boost::asio::ip::tcp get_tcp(const ip_version ip_version);
}