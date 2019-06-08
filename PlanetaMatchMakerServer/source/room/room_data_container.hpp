#pragma once

#include <unordered_map>
#include <atomic>
#include <shared_mutex>

#include <boost/noncopyable.hpp>

#include "room_data.hpp"

namespace pgl {
	// A thread safe container of room data
	class room_data_container final : boost::noncopyable {
	public:
		[[nodiscard]] bool is_room_data_exist(room_id_type room_id) const;
		[[nodiscard]] room_data get_room_data(room_id_type room_id) const;
		void add_room_data(room_id_type room_id, const room_data& room_data);
		void update_room_data(room_id_type room_id, const room_data& room_data);
	private:
		std::unordered_map<room_id_type, std::atomic<room_data>> room_data_map_;
		mutable std::shared_mutex mutex_;
	};
}
