#pragma once

#include <unordered_map>
#include <atomic>
#include <shared_mutex>
#include <functional>
#include <vector>
#include <algorithm>
#include <unordered_set>

#include <boost/noncopyable.hpp>
#include <boost/call_traits.hpp>

#include "random_id_generator.hpp"

namespace pgl {
	template <typename S, typename T>
	auto member_variable_pointer_t_impl(T S::* p) -> T;

	/**
	 * A type of member variable from a member variable pointer.
	 *
	 * @tparam T A member variable pointer of a member variable.
	 */
	template <typename T>
	using member_variable_pointer_variable_t = decltype(member_variable_pointer_t_impl(std::declval<T>()));

	/**
	 * A container to hold data and manage them with checking value of unique member duplication.
	 *
	 * @tparam IdType A type of ID value.
	 * @tparam S A type of data to hold.
	 * @tparam UniqueVariable A member variable pointer of the member variable in Data which should be unique in this container.
	 */
	template <typename IdType, typename S, auto S::* UniqueVariable>
	class unique_variables_container_impl {
		using member_t = member_variable_pointer_variable_t<decltype(UniqueVariable)>;
		using s_param_t = typename boost::call_traits<S>::param_type;
		using id_param_t = typename boost::call_traits<IdType>::param_type;
	protected:
		/**
		 * Add or update data with ID.
		 *
		 * @param id An ID of data to add or update.
		 * @param data A data to add or update.
		 */
		void add_or_update_variable(id_param_t id, s_param_t data) {
			auto unique_value = get_unique_value(data);
			used_variable_.insert(unique_value);
			id_to_variable_map_.emplace(id, unique_value);
		}

		/**
		 * Remove data with ID.
		 *
		 * @param id An ID of data to remove.
		 * @param data A data to remove.
		 */
		void remove_variable(id_param_t id, s_param_t data) {
			used_variable_.erase(get_unique_value(data));
			id_to_variable_map_.erase(id);
		}

		/**
		 * Check if indicated data is unique for all unique member variables with it's ID.
		 *
		 * @param id An ID of data to check.
		 * @param data A data to check.
		 * @return Whether indicated data is unique.
		 * @note If variable is same with variable of id, consider the variable as unique even if the variable is in used_variables.
		 */
		bool is_unique(id_param_t id, s_param_t data) const {
			auto unique_value = get_unique_value(data);
			auto it = id_to_variable_map_.find(id);
			if (it == id_to_variable_map_.end()) { return used_variable_.find(unique_value) == used_variable_.end(); }

			return it->second == unique_value;
		}

		/**
		 * Check if indicated data is unique for all unique member variables.
		 *
		 * @param data A data to check.
		 * @return Whether indicated data is unique.
		 */
		bool is_unique(s_param_t data) const {
			auto unique_value = get_unique_value(data);
			return used_variable_.find(unique_value) == used_variable_.end();
		}

	private:
		std::unordered_map<IdType, member_t> id_to_variable_map_;
		std::unordered_set<member_t> used_variable_;

		static member_t get_unique_value(s_param_t data) { return data.*UniqueVariable; }
	};


	/**
	 * A container to hold data and manage them with checking values of multiple unique members duplication.
	 *
	 * @tparam IdType A type of ID value.
	 * @tparam S A type of data to hold.
	 * @tparam UniqueVariables Member variable pointers of the member variables in Data which should be unique in this container.
	 */
	template <typename IdType, typename S, auto S::* ... UniqueVariables>
	class unique_variables_container
		final : boost::noncopyable, unique_variables_container_impl<IdType, S, UniqueVariables>... {
		using s_param_t = typename boost::call_traits<S>::param_type;
		using id_param_t = typename boost::call_traits<IdType>::param_type;
	public:
		/**
		 * Add or update data with ID.
		 *
		 * @param id An ID of data to add or update.
		 * @param data A data to add or update.
		 */
		void add_or_update_variables(id_param_t id, s_param_t data) {
			(unique_variables_container_impl<IdType, S, UniqueVariables>::add_or_update_variable(id, data), ...);
		}

		/**
		 * Remove data with ID.
		 *
		 * @param id An ID of data to remove.
		 * @param data A data to remove.
		 */
		void remove_variables(id_param_t id, s_param_t data) {
			(unique_variables_container_impl<IdType, S, UniqueVariables>::remove_variable(id, data), ...);
		}

		/**
		 * Check if indicated data is unique for all unique member variables with it's ID.
		 *
		 * @param id An ID of data to check.
		 * @param data A data to check.
		 * @return Whether indicated data is unique.
		 */
		bool is_unique(id_param_t id, s_param_t data) const {
			return (unique_variables_container_impl<IdType, S, UniqueVariables>::is_unique(id, data) && ... && true);
		}

		/**
		 * Check if indicated data is unique for all unique member variables.
		 *
		 * @param data A data to check.
		 * @return Whether indicated data is unique.
		 */
		bool is_unique(s_param_t data) const {
			return (unique_variables_container_impl<IdType, S, UniqueVariables>::is_unique(data) && ... && true);
		}
	};

	/**
	 * An exception class indicating value of unique member variable duplication error.
	 */
	class unique_variable_duplication_error final : public std::runtime_error {
	public:
		unique_variable_duplication_error() : runtime_error("A unique member variable is duplicated.") {};
		using runtime_error::runtime_error;
	};

	/**
	 * A container to hold data and manage them with id thread safely.
	 * 
	 * @tparam Id A type of ID value.
	 * @tparam Data A type of data to hold.
	 * @tparam UniqueVariables Member variable pointers of the member variables in Data which should be unique in this container.
	 */
	template <typename Id, typename Data, auto Data::* ... UniqueVariables>
	class thread_safe_data_container final : boost::noncopyable {
	public:
		using id_param_type = typename boost::call_traits<Id>::param_type;
		using data_param_type = typename boost::call_traits<Data>::param_type;

		/**
		 * Check data existence for specific ID.
		 * 
		 * @param id ID to check.
		 * @return Whether data exists for the ID.
		 */
		[[nodiscard]] bool is_data_exist(id_param_type id) const {
			std::shared_lock lock(mutex_);
			return data_map_.find(id) != data_map_.end();
		}

		/**
		 * Get data by ID.
		 * 
		 * @param id ID to get data.
		 * @return Data which has passed ID.
		 */
		[[nodiscard]] Data get_data(id_param_type id) const {
			std::shared_lock lock(mutex_);
			return data_map_.at(id).load();
		}

		/**
		 * Get the number of data.
		 *
		 * @return The number of data.
		 */
		size_t size() const { return data_map_.size(); }

		/**
		 * Add new data with indicated ID.
		 *
		 * @param id An ID for new data.
		 * @param data Data to add.
		 * @throw unique_variable_duplication_error Unique member variable is duplicated.
		 */
		void add_data(id_param_type id, data_param_type data) {
			std::lock_guard lock(mutex_);

			if (!unique_variables_.is_unique(data)) { throw unique_variable_duplication_error(); }

			data_map_.emplace(id, data);
			unique_variables_.add_or_update_variables(id, data);
		}

		/**
		 * Add new data with automatically assigned ID.
		 *
		 * @param data Data to add.
		 * @param id_setter (optional) A function to set assigned ID to the data. In default, assigned ID is not set to the data.
		 * @param random_id_generator (optional) A functions to generate new ID. In default, built-in random value generator will be used.
		 * @return An ID assigned to new data.
		 * @throw unique_variable_duplication_error Unique member variable is duplicated.
		 */
		Id assign_id_and_add_data(Data& data,
			std::function<void(Data&, id_param_type)>&& id_setter = [](Data&, id_param_type) {},
			std::function<Id()>&& random_id_generator = generate_random_id<Id>) {
			std::lock_guard lock(mutex_);

			if (!unique_variables_.is_unique(data)) { throw unique_variable_duplication_error(); }

			Id id;
			do { id = random_id_generator(); }
			while (data_map_.find(id) != data_map_.end());
			id_setter(data, id);
			data_map_.emplace(id, data);
			unique_variables_.add_or_update_variables(id, data);
			return id;
		}

		/**
		 * Get sorted and filtered data.
		 *
		 * @param compare_function A function used to sorting.
		 * @param filter_function A function used to filtering.
		 * @return A list of data.
		 */
		std::vector<Data> get_data(std::function<bool(data_param_type, data_param_type)>&& compare_function,
			std::function<bool(data_param_type)>&& filter_function) const {
			std::vector<Data> data;
			{
				std::shared_lock lock(mutex_);
				data.reserve(data_map_.size());
				for (auto&& pair : data_map_) {
					const auto data_elem = pair.second.load();
					if (filter_function(data_elem)) { data.push_back(data_elem); }
				}
			}
			std::sort(data.begin(), data.end(), compare_function);
			return data;
		}

		/**
		 * Get sorted and filtered data with indicating range.
		 *
		 * @param start_idx A start index of range.
		 * @param count The number of data in range.
		 * @param compare_function A function used to sorting.
		 * @param filter_function A function used to filtering.
		 * @return A list of data.
		 */
		std::vector<Data> get_range_data(const size_t start_idx, size_t count,
			std::function<bool(data_param_type, data_param_type)>&& compare_function,
			std::function<bool(data_param_type)>&& filter_function) const {
			std::vector<Data> data = get_data(std::move(compare_function), std::move(filter_function));
			count = std::min(count, data.size() >= start_idx ? data.size() - start_idx : 0);
			std::vector<Data> result(count);
			const auto end_idx_plus_one = start_idx + count;
			for (auto i = start_idx; i < end_idx_plus_one; ++i) { result[i - start_idx] = data[i]; }
			return result;
		}

		/**
		 * Update data of indicated ID.
		 *
		 * @param id An ID data you want to update.
		 * @param data New data.
		 * @throw unique_variable_duplication_error Unique member variable is duplicated.
		 */
		void update_data(id_param_type id, data_param_type data) {
			std::shared_lock lock(mutex_);

			if (!unique_variables_.is_unique(id, data)) { throw unique_variable_duplication_error(); }

			unique_variables_.add_or_update_variables(id, data);
			data_map_.at(id) = data;
		}

		/**
		 * Remove data of indicated ID.
		 *
		 * @param id An ID data you want to remove.
		 */
		void remove_data(id_param_type id) {
			std::lock_guard lock(mutex_);
			auto it = data_map_.find(id);
			unique_variables_.remove_variables(id, it->second);
			data_map_.erase(it);
		}

	private:
		std::unordered_map<Id, std::atomic<Data>> data_map_;
		unique_variables_container<Id, Data, UniqueVariables...> unique_variables_;
		mutable std::shared_mutex mutex_;
	};
}
