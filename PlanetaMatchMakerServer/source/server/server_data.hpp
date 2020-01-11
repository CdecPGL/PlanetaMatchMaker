#pragma once

#include <vector>

#include "room/room_data_container.hpp"
#include "client/player_name_container.hpp"

namespace pgl {
	class server_data final {
	public:
		using room_data_container_type = room_data_container;
		using room_group_data_list_type = std::vector<room_group_data>;

		explicit server_data(std::vector<room_group_name_t>&& room_groups);

		[[nodiscard]] size_t room_group_count() const;

		[[nodiscard]] bool is_valid_room_group_index(room_group_index_t room_group_index) const;

		[[nodiscard]] const room_group_data_list_type& get_room_data_group_list() const;

		[[nodiscard]] const room_data_container_type& get_room_data_container(room_group_index_t room_group_idx) const;

		[[nodiscard]] const player_name_container& get_player_name_container() const;

		[[nodiscard]] room_data_container_type& get_room_data_container(room_group_index_t room_group_idx);

		[[nodiscard]] player_name_container& get_player_name_container();
	private:
		room_group_data_list_type room_group_data_list_;
		std::vector<room_data_container_type> room_data_container_list_;
		player_name_container player_name_container_;
	};
}
