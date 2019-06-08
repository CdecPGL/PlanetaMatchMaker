#pragma once
#include "data/thread_safe_data_container.h"
#include "room/room_data.hpp"
#include "client/client_data.hpp"

namespace pgl {
	struct server_data final {
		const thread_safe_data_container<room_id_type, room_data> room_data_container{};
		const thread_safe_data_container<client_address, client_data> client_data_container{};
	};
}
