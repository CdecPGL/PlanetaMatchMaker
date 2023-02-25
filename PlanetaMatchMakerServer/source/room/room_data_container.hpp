#pragma once

#include "data/thread_safe_data_container.hpp"
#include "client/player_full_name.hpp"

#include "room_constants.hpp"
#include "room_data.hpp"

namespace pgl {
	/**
	 * A thread safe container of room data.
	 */
	class room_data_container final {
	public:
		using container_type = thread_safe_data_container<room_data, &room_data::room_id, &
			room_data::host_player_full_name>;
		using id_param_type = container_type::id_param_type;
		using data_param_type = container_type::data_param_type;

		/**
		 * Check if the room data exists with specific ID.
		 *
		 * @param id An ID to check existence.
		 * @return Whether the room exists.
		 */
		[[nodiscard]] bool contains(id_param_type id) const { return container_.contains(id); }

		/**
		 * Get a room data with specific ID.
		 *
		 * @param id An ID to get room data.
		 * @return A room data.
		 */
		[[nodiscard]] room_data get(id_param_type id) const { return container_.get(id); }

		/**
		 * Get the number of room data.
		 *
		 * @return The number of room data.
		 */
		[[nodiscard]] size_t size() const { return container_.size(); }

		/**
		 * Add new room data with ID assigned automatically.
		 *
		 * @param data New room data rvalue reference.
		 * @throw unique_variable_duplication_error Unique member variable is duplicate.
		 */
		room_id_t assign_id_and_add(room_data&& data) {
			return container_.assign_id_and_add(std::forward<room_data>(data));
		}

		/**
		 * Add new room data with ID assigned automatically.
		 *
		 * @param data New room data.
		 * @throw unique_variable_duplication_error Unique member variable is duplicate.
		 */
		room_id_t assign_id_and_add(const room_data& data) {
			return container_.assign_id_and_add(data);
		}

		/**
		 * Search room data which matches conditions.
		 *
		 * @param sort_kind A kind of sort for the result list. A room data which exactly matches search_full_name is always located top whatever sort kind is.
		 * @param search_target_flags A flags of condition to search rooms. Rooms whose status matches some more than or equals one flag will be returned.
		 * @param search_full_name A room full name to search. Not only full name ("Bill#123") but also tag ("#123"), name ("Bill") or empty string are available.
		 * @return A list of result room data.
		 * @throw std::out_of_range room_data_sort_kind is invalid.
		 */
		std::vector<room_data> search(const room_data_sort_kind sort_kind,
			const room_search_target_flag search_target_flags,
			const player_full_name& search_full_name) const {
			return container_.search(get_room_data_compare_function(sort_kind, search_full_name),
				get_room_data_filter_function(search_target_flags, search_full_name));
		}

		/**
		 * Search room data which matches conditions with range.
		 *
		 * @param start_idx A start index of range.
		 * @param count The number of data in range.
		 * @param sort_kind A kind of sort for the result list. A room data which exactly matches search_full_name is always located top whatever sort kind is.
		 * @param search_target_flags A flags of condition to search rooms. Rooms whose status matches some more than or equals one flag will be returned.
		 * @param search_full_name A room full name to search. Not only full name ("Bill#123") but also tag ("#123"), name ("Bill") or empty string are available.
		 * @return A list of result room data.
		 * @throw std::out_of_range room_data_sort_kind is invalid.
		 */
		std::vector<room_data> search_range(const int start_idx, const int count,
			const room_data_sort_kind sort_kind, const room_search_target_flag search_target_flags,
			const player_full_name& search_full_name) const {
			return container_.search_range(start_idx, count,
				get_room_data_compare_function(sort_kind, search_full_name),
				get_room_data_filter_function(search_target_flags, search_full_name));
		}

		/**
		 * Add or update room.
		 *
		 * @param data A room data rvalue reference to update.
		 * @throw unique_variable_duplication_error Unique member variable is duplicated.
		 * @return true if added.
		*/
		bool add_or_update(room_data&& data) { return container_.add_or_update(std::forward<room_data>(data)); }

		/**
		 * Add or update room.
		 *
		 * @param data A room data to update.
		 * @throw unique_variable_duplication_error Unique member variable is duplicated.
		 * @return true if added.
		*/
		bool add_or_update(const room_data& data) { return container_.add_or_update(data); }

		/**
		 * Remove room data with an ID.
		 *
		 * @param id An ID of room data to remove.
		 * @return true if removed.
		 */
		bool try_remove(id_param_type id) { return container_.try_remove(id); }

	private:
		container_type container_;
	};
}