#include "room_data_container.hpp"

namespace pgl {
	bool room_data_container::is_room_data_exist(const room_id_type room_id) const {
		std::shared_lock<decltype(mutex_)> lock(mutex_);
		return room_data_map_.find(room_id) != room_data_map_.end();
	}

	room_data room_data_container::get_room_data(const room_id_type room_id) const {
		std::shared_lock<decltype(mutex_)> lock(mutex_);
		return room_data_map_.at(room_id).load();
	}

	void room_data_container::add_room_data(room_id_type room_id, const room_data& room_data) {
		std::lock_guard<decltype(mutex_)> lock(mutex_);
		room_data_map_.emplace(room_id, room_data);
	}

	void room_data_container::update_room_data(const room_id_type room_id, const room_data& room_data) {
		std::shared_lock<decltype(mutex_)> lock(mutex_);
		room_data_map_.at(room_id) = room_data;
	}
}
