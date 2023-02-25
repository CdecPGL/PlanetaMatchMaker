#include "message_handle_utilities.hpp"

namespace pgl {
	bool does_room_exist(const std::shared_ptr<message_handle_parameter> param,
		const room_data_container& room_data_container, room_id_t room_id) {
		// Check room existence
		if (room_data_container.contains(room_id)) {
			log_with_endpoint(log_level::debug, param->socket.remote_endpoint(), "The room whose id is \"", room_id,
				"\" exists.");
			return true;
		}

		log_with_endpoint(log_level::error, param->socket.remote_endpoint(), "The room whose id is \"", room_id,
			"\" doesn't exist.");
		return false;
	}
}
