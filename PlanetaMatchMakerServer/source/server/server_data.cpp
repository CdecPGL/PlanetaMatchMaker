#include "server_data.hpp"

namespace pgl {
	const server_data::room_data_container_type& server_data::get_room_data_container() const {
		return room_data_container_;
	}

	const player_name_container& server_data::get_player_name_container() const { return player_name_container_; }

	server_data::room_data_container_type& server_data::
	get_room_data_container() { return room_data_container_; }

	player_name_container& server_data::get_player_name_container() { return player_name_container_; }
}
