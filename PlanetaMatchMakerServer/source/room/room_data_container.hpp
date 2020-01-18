#pragma once

#include "data/thread_safe_data_container.hpp"
#include "client/player_full_name.hpp"

#include "room_constants.hpp"
#include "room_data.hpp"

namespace pgl {
	// A thread safe container of room data
	class room_data_container final {
	public:
		using container_type = thread_safe_data_container<room_id_t, room_data, &room_data::host_player_full_name>;
		using id_param_type = container_type::id_param_type;
		using data_param_type = container_type::data_param_type;

		[[nodiscard]] bool is_data_exist(id_param_type id) const { return container_.is_data_exist(id); }

		[[nodiscard]] room_data get_data(id_param_type id) const { return container_.get_data(id); }

		size_t size() const { return container_.size(); }

		// unique_variable_duplication_error will be thrown if unique member variable is duplicated.
		void add_data(data_param_type data) { container_.add_data(data.room_id, data); }

		// unique_variable_duplication_error will be thrown if unique member variable is duplicated.
		room_id_t assign_id_and_add_data(room_data& data) {
			return container_.assign_id_and_add_data(data, [](room_data& data, id_param_type id) {
				data.room_id = id;
			});
		}

		// Throws std::out_of_range if room_data_sort_kind is invalid.
		std::vector<room_data> get_data(const room_data_sort_kind sort_kind,
			const room_search_target_flag search_target_flags,
			const player_full_name& search_full_name) const {
			return container_.get_data(get_room_data_compare_function(sort_kind, search_full_name),
				get_room_data_filter_function(search_target_flags, search_full_name));
		}

		// Throws std::out_of_range if room_data_sort_kind is invalid.
		std::vector<room_data> get_range_data(const int start_idx, const int count,
			const room_data_sort_kind sort_kind, const room_search_target_flag search_target_flags,
			const player_full_name& search_full_name) const {
			return container_.get_range_data(start_idx, count,
				get_room_data_compare_function(sort_kind, search_full_name),
				get_room_data_filter_function(search_target_flags, search_full_name));
		}

		// unique_variable_duplication_error will be thrown if unique member variable is duplicated.
		void update_data(data_param_type data) { container_.update_data(data.room_id, data); }

		void remove_data(id_param_type id) { container_.remove_data(id); }

	private:
		container_type container_;
	};
}
