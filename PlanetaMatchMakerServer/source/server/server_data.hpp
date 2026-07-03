#pragma once

#include <atomic>

#include "room/room_data_container.hpp"
#include "client/player_name_container.hpp"
#include "session/session_constants.hpp"

namespace pgl {
	class server_data final {
	public:
		using room_data_container_type = room_data_container;

		[[nodiscard]] const room_data_container_type& get_room_data_container() const;

		[[nodiscard]] const player_name_container& get_player_name_container() const;

		[[nodiscard]] room_data_container_type& get_room_data_container();

		[[nodiscard]] player_name_container& get_player_name_container();

		[[nodiscard]] session_number_t issue_session_number();
	private:
		std::atomic<session_number_t> next_session_number_{1};
		room_data_container_type room_data_container_;
		player_name_container player_name_container_;
	};
}
