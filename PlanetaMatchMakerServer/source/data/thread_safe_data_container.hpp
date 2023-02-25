#pragma once

#include <unordered_map>
#include <atomic>
#include <shared_mutex>
#include <functional>
#include <vector>
#include <algorithm>

#include <boost/noncopyable.hpp>
#include <boost/call_traits.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/hashed_index.hpp>

#include "utilities/class_traits.hpp"
#include "random_id_generator.hpp"

namespace pgl {
	/**
	 * A container to hold data and manage them with checking values of multiple unique members duplication.
	 *
	 * @tparam Data A type of data to hold.
	 * @tparam IdMemberVariable A member variable pointer of ID value.
	 * @tparam UniqueMemberVariables Member variable pointers of the member variables in Data which should be unique in this container.
	 */
	template <typename Data, auto Data::* IdMemberVariable, auto Data::*... UniqueMemberVariables>
	class unique_variables_container final {
		template <auto Data::* P>
		using multi_index_hashed_unique_member_t = boost::multi_index::hashed_unique<
			boost::multi_index::member<
				Data,
				member_variable_pointer_variable_t<P>,
				P
			>
		>;

		using container_type = boost::multi_index_container<
			std::reference_wrapper<const Data>,
			boost::multi_index::indexed_by<
				multi_index_hashed_unique_member_t<IdMemberVariable>,
				multi_index_hashed_unique_member_t<UniqueMemberVariables>...
			>
		>;
		using id_type = member_variable_pointer_variable_t<IdMemberVariable>;
		using id_param_type = typename boost::call_traits<id_type>::param_type;

	public:
		/**
		 * Add or update data with ID.
		 *
		 * @param data A data to add or update.
		 */
		void add_or_update_variables(const Data& data) { data_.emplace(data); }

		/**
		 * Remove data with ID.
		 *
		 * @param id An ID of data to remove.
		 */
		void remove_variables(id_param_type id) { data_.erase(id); }

		/**
		 * Check if indicated data is unique for id and all unique member variables.
		 * Duplication with data whose id is same as the id of passed data is ignored.
		 *
		 * @param data A data to check.
		 * @return Whether indicated data is unique.
		 */
		[[nodiscard]] bool is_unique(const Data& data) const { return is_unique_in_all_indexes(data); }

	private:
		container_type data_;
		constexpr static size_t unique_variable_count = 1 + sizeof...(UniqueMemberVariables);

		template <size_t Index = 0> requires(Index < unique_variable_count)
		[[nodiscard]] bool is_unique_in_all_indexes(const Data& data) const {
			constexpr auto p = std::get<Index>(std::tuple(IdMemberVariable, UniqueMemberVariables...));
			const auto& v = data.*p;
			const auto& c = data_.template get<Index>();
			const auto it = c.find(v);
			auto is_unique = it == c.end();
			// Ignore duplication with myself 
			if (!is_unique && it->get().*IdMemberVariable == data.*IdMemberVariable) { is_unique = true; }
			return is_unique_in_all_indexes<Index + 1>(data) && is_unique;
		}

		template <size_t Index> requires(Index >= unique_variable_count)
		[[nodiscard]] static bool is_unique_in_all_indexes(const Data&) { return true; }
	};

	/**
	 * An exception class indicating value of unique member variable duplication error.
	 */
	class unique_variable_duplication_error final : public std::runtime_error {
	public:
		unique_variable_duplication_error() : runtime_error("A unique member variable is duplicated.") {}
		using runtime_error::runtime_error;
	};

	/**
	 * A container to hold data and manage them with id thread safely.
	 * 
	 * @tparam Data A type of data to hold.
	 * @tparam IdMemberVariable A member variable pointer of ID value.
	 * @tparam UniqueMemberVariables Member variable pointers of the member variables in Data which should be unique in this container.
	 */
	template <typename Data, auto Data::* IdMemberVariable, auto Data::*... UniqueMemberVariables>
	class thread_safe_data_container final : boost::noncopyable {
	public:
		using id_type = member_variable_pointer_variable_t<IdMemberVariable>;
		using id_param_type = typename boost::call_traits<id_type>::param_type;
		using data_param_type = typename boost::call_traits<Data>::param_type;

		/**
		 * Add or update data.
		 *
		 * @param data New data of rvalue reference.
		 * @throw unique_variable_duplication_error Unique member variable is duplicated.
		 * @return true if added.
		 */
		bool add_or_update(Data&& data) {
			const auto id = get_id(data);
			std::shared_lock lock(mutex_);

			if (!unique_variables_.is_unique(data)) { throw unique_variable_duplication_error(); }

			auto&& [it, is_added] = data_map_.insert_or_assign(id, data);
			unique_variables_.add_or_update_variables(it->second);
			return is_added;
		}

		/**
		 * Update data of indicated ID.
		 *
		 * @param data New data.
		 * @throw unique_variable_duplication_error Unique member variable is duplicated.
		 * @return true if added.
		 */
		bool add_or_update(const Data& data) { return add_or_update(Data{data}); }

		/**
		 * Add new data with automatically assigned ID.
		 *
		 * @param data Data of rvalue reference
		 * @param random_id_generator (optional) A functions to generate new ID. In default, built-in random value generator will be used.
		 * @return An ID assigned to new data.
		 * @throw unique_variable_duplication_error Unique member variable is duplicated.
		 */
		id_type assign_id_and_add(Data&& data,
			std::function<id_type()>&& random_id_generator = generate_random_id<id_type>) {
			std::lock_guard lock(mutex_);

			id_type id{};
			do { id = random_id_generator(); }
			while (data_map_.contains(id));
			data.*IdMemberVariable = id;

			// Check if unique because ensure id of passed data is not duplicate existing id
			if (!unique_variables_.is_unique(data)) { throw unique_variable_duplication_error(); }

			auto&& [it,_] = data_map_.emplace(id, data);
			unique_variables_.add_or_update_variables(it->second);
			return id;
		}

		/**
		 * Add new data with automatically assigned ID.
		 *
		 * @param data Data to add.
		 * @param random_id_generator (optional) A functions to generate new ID. In default, built-in random value generator will be used.
		 * @return An ID assigned to new data.
		 * @throw unique_variable_duplication_error Unique member variable is duplicated.
		 */
		id_type assign_id_and_add(const Data& data,
			std::function<id_type()>&& random_id_generator = generate_random_id<id_type>) {
			return assign_id_and_add(Data{data}, std::move(random_id_generator));
		}

		/**
		 * Get data by ID.
		 * 
		 * @param id ID to get data.
		 * @throw std::out_of_range A data with passed ID does not exist.
		 * @return Data which has passed ID.
		 */
		[[nodiscard]] Data get(id_param_type id) const {
			std::shared_lock lock(mutex_);
			return data_map_.at(id).load();
		}

		/**
		 * Search data by filter and return sorted result.
		 *
		 * @param compare_function A function used to sort.
		 * @param filter_function A function used to filter.
		 * @return A list of data.
		 */
		[[nodiscard]] std::vector<Data> search(
			std::function<bool(data_param_type, data_param_type)>&& compare_function,
			std::function<bool(data_param_type)>&& filter_function) const {
			std::vector<Data> data;
			{
				std::shared_lock lock(mutex_);
				data.reserve(data_map_.size());
				for (auto&& pair : data_map_) {
					if (const auto data_elem = pair.second.load(); filter_function(data_elem)) {
						data.push_back(data_elem);
					}
				}
			}
			std::sort(data.begin(), data.end(), compare_function);
			return data;
		}

		/**
		 * Search data by filter and return sorted result with indicating range.
		 *
		 * @param start_idx A start index of range.
		 * @param count The number of data in range.
		 * @param compare_function A function used to sort.
		 * @param filter_function A function used to filter.
		 * @return A list of data.
		 */
		[[nodiscard]] std::vector<Data> search_range(const size_t start_idx, size_t count,
			std::function<bool(data_param_type, data_param_type)>&& compare_function,
			std::function<bool(data_param_type)>&& filter_function) const {
			std::vector<Data> data = search(std::move(compare_function), std::move(filter_function));
			count = std::min(count, data.size() >= start_idx ? data.size() - start_idx : 0);
			std::vector<Data> result(count);
			const auto end_idx_plus_one = start_idx + count;
			for (auto i = start_idx; i < end_idx_plus_one; ++i) { result[i - start_idx] = data[i]; }
			return result;
		}

		/**
		 * Check data existence for specific ID.
		 *
		 * @param id ID to check.
		 * @return Whether data exists for the ID.
		 */
		[[nodiscard]] bool contains(id_param_type id) const {
			std::shared_lock lock(mutex_);
			return data_map_.contains(id);
		}

		/**
		 * Remove data of indicated ID.
		 *
		 * @param id An ID data you want to remove.
		 * @return true if removed.
		 */
		bool try_remove(id_param_type id) {
			std::lock_guard lock(mutex_);
			unique_variables_.remove_variables(id);
			return data_map_.erase(id) == 1;
		}

		/**
		 * Get the number of data.
		 *
		 * @return The number of data.
		 */
		[[nodiscard]] size_t size() const { return data_map_.size(); }

	private:
		std::unordered_map<id_type, std::atomic<Data>> data_map_;
		unique_variables_container<Data, IdMemberVariable, UniqueMemberVariables...> unique_variables_;
		mutable std::shared_mutex mutex_;

		static id_type get_id(const data_param_type data) { return data.*IdMemberVariable; }
	};
}