#pragma once

#include "datetime/datetime.hpp"
#include "message/messages.hpp"

namespace pgl {
	struct client_data final{
		client_address address{};
		datetime last_communicate_datetime{};
	};
}
