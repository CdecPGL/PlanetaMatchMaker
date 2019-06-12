#pragma once

#include <vector>

#include "data/thread_safe_data_container.h"
#include "room/room_data_container.hpp"
#include "client/client_data.hpp"

namespace pgl {
	class server_data final {
	public:
		using client_data_container_type = thread_safe_data_container<client_address, client_data>;
		using room_data_container_type = room_data_container;
		using room_group_data_list_type = std::vector<room_group_data>;

		explicit
		server_data(std::vector<room_group_name_type>&& room_groups);

		size_t room_group_count()const;

		bool is_valid_room_group_index(size_t room_group_index)const;

		const room_group_data_list_type& get_room_data_group_list() const;

		const room_data_container_type& get_room_data_container(size_t room_group_idx) const;

		room_data_container_type& get_room_data_container(size_t room_group_idx);

		const client_data_container_type& client_data_container() const;

		client_data_container_type& client_data_container();

	private:
		room_group_data_list_type room_group_data_list_;
		std::vector<room_data_container_type> room_data_container_list_;
		client_data_container_type client_data_container_;
	};
}
