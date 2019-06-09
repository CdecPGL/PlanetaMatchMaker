#pragma once

#include "data/thread_safe_data_container.h"

#include "room_constants.hpp"
#include "room_data.hpp"

namespace pgl {
	class room_data_container final : public thread_safe_data_container<room_id_type, room_data> {
	public:
	};
}
