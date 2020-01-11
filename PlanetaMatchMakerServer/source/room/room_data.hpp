#pragma once
#include <cstdint>

#include "datetime/datetime.hpp"
#include "network/endpoint.hpp"
#include "utilities/enum_utilities.hpp"
#include "client/player_full_name.hpp"

#include "room_constants.hpp"

namespace pgl {
	enum class room_setting_flag : uint8_t {
		none = 0,
		public_room = 1,
		open_room = 2
	};

	namespace enum_concept {
		template <>
		struct has_bitwise_operators<room_setting_flag> : std::true_type {};
	}

	struct room_data final {
		room_id_t room_id;
		player_full_name host_player_full_name;
		room_setting_flag setting_flags;
		room_password_t password;
		uint8_t max_player_count;
		datetime create_datetime;
		endpoint host_endpoint;
		endpoint game_host_endpoint;
		uint8_t current_player_count;
	};

	enum class room_search_target_flag : uint8_t {
		none = 0,
		public_room = 1,
		private_room = 2,
		open_room = 4,
		closed_room = 8
	};

	namespace enum_concept {
		template <>
		struct has_bitwise_operators<room_search_target_flag> : std::true_type {};
	}

	// 24 bytes
	struct room_group_data final {
		room_group_name_t name;
	};

	enum class room_data_sort_kind : uint8_t {
		name_ascending,
		name_descending,
		create_datetime_ascending,
		create_datetime_descending
	};

	// The room whose name matches search name comes top if search name if not empty
	std::function<bool(const room_data&, const room_data&)> get_room_data_compare_function(
		room_data_sort_kind sort_kind, const player_full_name& search_full_name);

	std::function<bool(const room_data&)> get_room_data_filter_function(room_search_target_flag search_target_flags,
		const player_full_name& search_full_name);

	std::ostream& operator <<(std::ostream& os, const room_data& room_data);
	std::ostream& operator <<(std::ostream& os, const room_group_data& room_group_data);
}
