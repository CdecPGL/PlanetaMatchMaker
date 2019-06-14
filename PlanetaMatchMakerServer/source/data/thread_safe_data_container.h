#pragma once

#include <unordered_map>
#include <atomic>
#include <shared_mutex>
#include <functional>
#include <vector>
#include <algorithm>

#include <boost/noncopyable.hpp>
#include <boost/call_traits.hpp>

#include "random_id_generator.hpp"

namespace pgl {
	// A thread safe container of data
	template <typename Id, typename Data>
	class thread_safe_data_container final : boost::noncopyable {
	public:
		using id_param_type = typename boost::call_traits<Id>::param_type;
		using data_param_type = typename boost::call_traits<Data>::param_type;

		[[nodiscard]] bool is_data_exist(id_param_type id) const {
			std::shared_lock lock(mutex_);
			return data_map_.find(id) != data_map_.end();
		}

		[[nodiscard]] Data get_data(id_param_type id) const {
			std::shared_lock lock(mutex_);
			return data_map_.at(id).load();
		}

		size_t size() const {
			return data_map_.size();
		}

		void add_data(id_param_type id, data_param_type data) {
			std::lock_guard lock(mutex_);
			data_map_.emplace(id, data);
		}

		Id assign_id_and_add_data(Data& data,
			std::function<void(Data&, id_param_type)>&& id_setter = [](Data&, id_param_type) {},
			std::function<Id()>&& random_id_generator = generate_random_id<Id>) {
			std::lock_guard lock(mutex_);
			Id id;
			do {
				id = random_id_generator();
			} while (data_map_.find(id) != data_map_.end());
			id_setter(data, id);
			data_map_.emplace(id, data);
			return id;
		}

		std::vector<Data> get_range_data(const size_t start_idx, size_t count,
			std::function<bool(data_param_type, data_param_type)>&& compare_function) const {
			std::shared_lock lock(mutex_);
			std::vector<Data> data;
			data.reserve(data_map_.size());
			for (auto&& pair : data_map_) {
				data.push_back(pair.second.load());
			}
			std::sort(data.begin(), data.end(), compare_function);
			count = std::min(count, data.size() - start_idx - 1);
			std::vector<Data> result(count);
			const auto end_idx = start_idx + count - 1;
			for (auto i = start_idx; i <= end_idx; ++i) {
				result[i - start_idx] = data[i];
			}
			return result;
		}

		void update_data(id_param_type id, data_param_type data) {
			std::shared_lock lock(mutex_);
			data_map_.at(id) = data;
		}

		void remove_data(id_param_type id) {
			std::lock_guard lock(mutex_);
			data_map_.erase(data_map_.find(id));
		}

	private:
		std::unordered_map<Id, std::atomic<Data>> data_map_;
		mutable std::shared_mutex mutex_;
	};
}
