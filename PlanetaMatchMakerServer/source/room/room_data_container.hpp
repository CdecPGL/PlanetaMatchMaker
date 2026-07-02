#pragma once

#include <algorithm>
#include <mutex>
#include <optional>
#include <unordered_map>
#include <utility>
#include <vector>

#include "data/thread_safe_data_container.hpp"
#include "client/player_full_name.hpp"

#include "room_constants.hpp"
#include "room_data.hpp"

namespace pgl {
	template <class T>
	concept room_data_repository = requires(T t)
	{
		{ t.contains(room_id_t()) } -> std::convertible_to<bool>;
		{ t.get(room_id_t()) } -> std::convertible_to<room_data>;
		{ t.try_get(room_id_t()) } -> std::convertible_to<std::optional<room_data>>;
		{ t.size() } -> std::convertible_to<size_t>;
		{ t.assign_id_and_add(room_data()) } -> std::convertible_to<room_id_t>;
		{ t.assign_id_and_add(std::declval<room_data>()) } -> std::convertible_to<room_id_t>;
		{ t.try_assign_id_and_add(room_data(), size_t()) } -> std::convertible_to<std::optional<room_id_t>>;
		{
			t.search(room_data_sort_kind(), room_search_target_flag(), player_full_name())
		} -> std::convertible_to<std::vector<room_data>>;
		{ t.remove(room_id_t()) } -> std::convertible_to<void>;
	};

	/**
	 * A thread safe container of room data.
	 */
	class room_data_container final {
	public:
		using container_type = thread_safe_data_container<room_data, &room_data::room_id, &
			room_data::host_player_full_name>;
		using id_type = container_type::id_type;
		using id_param_type = container_type::id_param_type;
		using data_param_type = container_type::data_param_type;

		enum class join_room_result {
			accepted,
			room_not_found,
			connection_establish_mode_mismatch,
			room_closed,
			password_wrong,
			room_full
		};

		struct join_room_result_data final {
			join_room_result result;
			std::optional<room_data> room;
		};

		struct search_result final {
			std::vector<room_data> data;
			size_t total_room_count;
		};

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
		 * Get a room data with specific ID if it exists.
		 *
		 * @param id An ID to get room data.
		 * @return A room data. std::nullopt if the room does not exist.
		 */
		[[nodiscard]] std::optional<room_data> try_get(id_param_type id) const { return container_.try_get(id); }

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
		room_id_t assign_id_and_add(const room_data& data) { return container_.assign_id_and_add(data); }

		/**
		 * Add new room data with ID assigned automatically only if the current room count is below max_size.
		 *
		 * @param data New room data rvalue reference.
		 * @param max_size Maximum number of rooms allowed.
		 * @return An ID assigned to new room data. std::nullopt if room count already reached max_size.
		 * @throw unique_variable_duplication_error Unique member variable is duplicate.
		 */
		std::optional<room_id_t> try_assign_id_and_add(room_data&& data, const size_t max_size) {
			return container_.try_assign_id_and_add(std::forward<room_data>(data), max_size);
		}

		/**
		 * Add new room data with ID assigned automatically only if the current room count is below max_size.
		 *
		 * @param data New room data.
		 * @param max_size Maximum number of rooms allowed.
		 * @return An ID assigned to new room data. std::nullopt if room count already reached max_size.
		 * @throw unique_variable_duplication_error Unique member variable is duplicate.
		 */
		std::optional<room_id_t> try_assign_id_and_add(const room_data& data, const size_t max_size) {
			return container_.try_assign_id_and_add(data, max_size);
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
		 * Search room data and return the total room count from the same locked snapshot.
		 *
		 * @param sort_kind A kind of sort for the result list. A room data which exactly matches search_full_name is always located top whatever sort kind is.
		 * @param search_target_flags A flags of condition to search rooms. Rooms whose status matches some more than or equals one flag will be returned.
		 * @param search_full_name A room full name to search. Not only full name ("Bill#123") but also tag ("#123"), name ("Bill") or empty string are available.
		 * @return A list of result room data and the total room count at the time the list was collected.
		 * @throw std::out_of_range room_data_sort_kind is invalid.
		 */
		search_result search_with_total(const room_data_sort_kind sort_kind,
			const room_search_target_flag search_target_flags,
			const player_full_name& search_full_name) const {
			auto result = container_.search_with_total(get_room_data_compare_function(sort_kind, search_full_name),
				get_room_data_filter_function(search_target_flags, search_full_name));
			return { std::move(result.data), result.total_count };
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
		bool add_or_update(room_data&& data) {
			const auto id = data.room_id;
			std::lock_guard lock(reservation_mutex_);
			const auto result = container_.add_or_update(std::forward<room_data>(data));
			reserved_player_count_map_.erase(id);
			return result;
		}

		/**
		 * Add or update room.
		 *
		 * @param data A room data to update.
		 * @throw unique_variable_duplication_error Unique member variable is duplicated.
		 * @return true if added.
		*/
		bool add_or_update(const room_data& data) { return add_or_update(room_data{data}); }

		/**
		 * Update an existing room under one exclusive lock.
		 *
		 * @param id An ID of room data to update.
		 * @param update_function A function to update copied room data.
		 * @return Updated room data. std::nullopt if the room does not exist.
		 * @throw unique_variable_duplication_error Unique member variable is duplicated.
		*/
		template <typename UpdateFunction>
		std::optional<room_data> try_update(id_param_type id, UpdateFunction&& update_function) {
			return container_.try_update(id, std::forward<UpdateFunction>(update_function));
		}

		/**
		 * Reserve one player slot for joining a room under one exclusive lock.
		 *
		 * @param id An ID of room data to join.
		 * @param connection_establish_mode An expected way how to establish P2P connection.
		 * @param password A password of the room. This is only referred when indicated room is private.
		 * @return Join result and a room snapshot. Accepted result contains the updated snapshot.
		 */
		join_room_result_data try_reserve_player_for_join(id_param_type id,
			const game_host_connection_establish_mode connection_establish_mode,
			const room_password_t& password) {
			std::lock_guard lock(reservation_mutex_);
			auto result = join_room_result_data{ join_room_result::room_not_found, std::nullopt };
			const auto updated_room_data = container_.try_update(id, [&](auto& target_room_data) {
				if (target_room_data.game_host_connection_establish_mode != connection_establish_mode) {
					result = { join_room_result::connection_establish_mode_mismatch, target_room_data };
					return;
				}

				if ((target_room_data.setting_flags & room_setting_flag::open_room) != room_setting_flag::open_room) {
					result = { join_room_result::room_closed, target_room_data };
					return;
				}

				if ((target_room_data.setting_flags & room_setting_flag::public_room) != room_setting_flag::public_room &&
					target_room_data.password != password) {
					result = { join_room_result::password_wrong, target_room_data };
					return;
				}

				if (target_room_data.current_player_count >= target_room_data.max_player_count) {
					result = { join_room_result::room_full, target_room_data };
					return;
				}

				// Reserve capacity immediately until a later host status notice confirms the joining player.
				++target_room_data.current_player_count;
				++reserved_player_count_map_[id];
				result = { join_room_result::accepted, target_room_data };
			});

			if (!updated_room_data.has_value()) { return result; }
			if (result.result == join_room_result::accepted) { result.room = updated_room_data; }
			return result;
		}

		/**
		 * Update a room and apply a player count reported by the host without dropping in-flight join reservations.
		 *
		 * The server increments current_player_count before the joining client reaches the host. A host status notice
		 * generated from an older snapshot must not erase that reservation, otherwise concurrent join requests can be
		 * over-accepted.
		 */
		template <typename UpdateFunction>
		std::optional<room_data> try_update_with_host_reported_current_player_count(id_param_type id,
			const bool is_current_player_count_changed, const uint8_t host_current_player_count,
			UpdateFunction&& update_function) {
			std::lock_guard lock(reservation_mutex_);
			return container_.try_update(id, [&](auto& target_room_data) {
				update_function(target_room_data);
				if (is_current_player_count_changed) {
					apply_host_reported_current_player_count(id, target_room_data, host_current_player_count);
				}
			});
		}

		/**
		 * Remove room data with an ID.
		 *
		 * @param id An ID of room data to remove.
		 * @return true if removed.
		 */
		bool try_remove(id_param_type id) {
			std::lock_guard lock(reservation_mutex_);
			const auto result = container_.try_remove(id);
			if (result) { reserved_player_count_map_.erase(id); }
			return result;
		}

		/**
		 * Remove room data with an ID under one exclusive lock if remove_function allows it.
		 *
		 * @param id An ID of room data to remove.
		 * @param remove_function A function to check copied room data.
		 * @return Removed room data. std::nullopt if the room does not exist or remove_function returns false.
		 */
		template <typename RemoveFunction>
		std::optional<room_data> try_remove_if(id_param_type id, RemoveFunction&& remove_function) {
			std::lock_guard lock(reservation_mutex_);
			auto result = container_.try_remove_if(id, std::forward<RemoveFunction>(remove_function));
			if (result.has_value()) { reserved_player_count_map_.erase(id); }
			return result;
		}

	private:
		container_type container_;
		std::unordered_map<id_type, uint8_t> reserved_player_count_map_;
		mutable std::mutex reservation_mutex_;

		void apply_host_reported_current_player_count(id_param_type id, room_data& target_room_data,
			const uint8_t host_current_player_count) {
			const auto reported_player_count = std::min<uint16_t>(host_current_player_count,
				target_room_data.max_player_count);
			auto& stored_reservation_count = reserved_player_count_map_[id];
			const auto reservation_count = std::min<uint16_t>(stored_reservation_count,
				target_room_data.current_player_count);
			stored_reservation_count = static_cast<uint8_t>(reservation_count);
			const auto previous_host_count = static_cast<uint16_t>(target_room_data.current_player_count) -
				reservation_count;

			if (reported_player_count > previous_host_count) {
				const auto confirmed_count = std::min<uint16_t>(
					reservation_count, reported_player_count - previous_host_count);
				stored_reservation_count -= static_cast<uint8_t>(confirmed_count);
			}

			const auto effective_player_count = std::min<uint16_t>(
				target_room_data.max_player_count,
				reported_player_count + stored_reservation_count);
			target_room_data.current_player_count = static_cast<uint8_t>(effective_player_count);
			if (stored_reservation_count == 0) { reserved_player_count_map_.erase(id); }
		}
	};
}
