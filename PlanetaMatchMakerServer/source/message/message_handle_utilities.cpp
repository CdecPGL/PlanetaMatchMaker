#include "message_handle_utilities.hpp"

namespace pgl {
	bool does_room_group_exist(const std::shared_ptr<message_handle_parameter> param,
		room_group_index_t room_group_index) {
		// Check if the id is valid
		if (param->server_data.is_valid_room_group_index(room_group_index)) {
			log_with_endpoint(log_level::debug, param->socket.remote_endpoint(), "The room group index \"",
				room_group_index, "\" exists.");
			return true;
		}

		// Send room group index doesn't exist error to the client
		log_with_endpoint(log_level::error, param->socket.remote_endpoint(), "The room group index \"",
			room_group_index, "\" doesn't exist. Range of valid room group index is 0 to ",
			param->server_data.room_group_count(), ".");
		return false;
	}

	bool does_room_exist(const std::shared_ptr<message_handle_parameter> param,
		const room_data_container& room_data_container, room_id_t room_id) {
		// Check room existence
		if (room_data_container.is_data_exist(room_id)) {
			log_with_endpoint(log_level::debug, param->socket.remote_endpoint(), "The room whose id is \"", room_id,
				"\" exists.");
			return true;
		}

		log_with_endpoint(log_level::error, param->socket.remote_endpoint(), "The room whose id is \"", room_id,
			"\" doesn't exist.");
		return false;
	}
}
