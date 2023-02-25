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
	template <typename D, auto P>
	using multi_index_hashed_unique_member_t = boost::multi_index::hashed_unique<
		boost::multi_index::member<
			D,
			member_variable_pointer_variable_t<P>,
			P
		>
	>;

	/**
	 * A container to hold data and manage them with checking values of multiple unique members duplication.
	 *
	 * @tparam Data A type of data to hold.
	 * @tparam IdMemberVariable A member variable pointer of ID value.
	 * @tparam UniqueMemberVariables Member variable pointers of the member variables in Data which should be unique in this container.
	 */
	template <typename Data, auto Data::* IdMemberVariable, auto Data::*... UniqueMemberVariables>
	class unique_variables_container final {
		using container_type = boost::multi_index_container<
			std::reference_wrapper<const Data>,
			boost::multi_index::indexed_by<
				multi_index_hashed_unique_member_t<Data, IdMemberVariable>,
				multi_index_hashed_unique_member_t<Data, UniqueMemberVariables>...
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
		 * Check if indicated data is unique for all unique member variables.
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
			auto is_unique = c.find(v) == c.end();
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
		 * Check data existence for specific ID.
		 * 
		 * @param id ID to check.
		 * @return Whether data exists for the ID.
		 */
		[[nodiscard]] bool is_data_exist(id_param_type id) const {
			std::shared_lock lock(mutex_);
			return data_map_.contains(id);
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
		[[nodiscard]] size_t size() const { return data_map_.size(); }

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
			unique_variables_.add_or_update_variables(data);
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
		id_type assign_id_and_add_data(Data& data,
			std::function<void(Data&, id_param_type)>&& id_setter = [](Data&, id_param_type) {},
			std::function<id_type()>&& random_id_generator = generate_random_id<id_type>) {
			std::lock_guard lock(mutex_);

			if (!unique_variables_.is_unique(data)) { throw unique_variable_duplication_error(); }

			id_type id{};
			do { id = random_id_generator(); }
			while (data_map_.contains(id));
			id_setter(data, id);
			data_map_.emplace(id, data);
			unique_variables_.add_or_update_variables(data);
			return id;
		}

		/**
		 * Get sorted and filtered data.
		 *
		 * @param compare_function A function used to sorting.
		 * @param filter_function A function used to filtering.
		 * @return A list of data.
		 */
		[[nodiscard]] std::vector<Data> get_data(
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
		 * Get sorted and filtered data with indicating range.
		 *
		 * @param start_idx A start index of range.
		 * @param count The number of data in range.
		 * @param compare_function A function used to sorting.
		 * @param filter_function A function used to filtering.
		 * @return A list of data.
		 */
		[[nodiscard]] std::vector<Data> get_range_data(const size_t start_idx, size_t count,
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

			if (!unique_variables_.is_unique(data)) { throw unique_variable_duplication_error(); }

			unique_variables_.add_or_update_variables(data);
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
			unique_variables_.remove_variables(id);
			data_map_.erase(it);
		}

	private:
		std::unordered_map<id_type, std::atomic<Data>> data_map_;
		unique_variables_container<Data, IdMemberVariable, UniqueMemberVariables...> unique_variables_;
		mutable std::shared_mutex mutex_;
	};
}