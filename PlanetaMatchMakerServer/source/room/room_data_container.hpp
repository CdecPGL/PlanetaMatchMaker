#pragma once

#include "data/thread_safe_data_container.h"

#include "room_constants.hpp"
#include "room_data.hpp"

namespace pgl {
	// A thread safe container of room data
	class room_data_container final {
		using container_type = thread_safe_data_container<room_id_type, room_data>;
		using id_param_type = container_type::id_param_type;
		using data_param_type = container_type::data_param_type;
	public:
		[[nodiscard]] bool is_data_exist(id_param_type id) const {
			return container_.is_data_exist(id);
		}

		[[nodiscard]] room_data get_data(id_param_type id) const {
			return container_.get_data(id);
		}

		void add_data(id_param_type id, data_param_type data) {
			container_.add_data(id, data);
		}

		room_id_type assign_id_and_add_data(room_data& data) {
			return container_.assign_id_and_add_data(data, [](room_data& data, id_param_type id)
			{
				data.room_id = id;
			});
		}

		/*std::vector<Data> get_data_range(int start_idx, int count, std::function<bool()>&& compare_function) const {
			std::shared_lock lock(mutex_);

		}*/

		void update_data(id_param_type id, data_param_type data) {
			container_.update_data(id, data);
		}

		void remove_data(id_param_type id) {
			container_.remove_data(id);
		}

	private:
		container_type container_;
	};
}
