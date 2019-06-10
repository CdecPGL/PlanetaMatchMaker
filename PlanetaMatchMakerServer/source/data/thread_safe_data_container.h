#pragma once

#include <unordered_map>
#include <atomic>
#include <shared_mutex>
#include <functional>
#include <vector>

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

		/*std::vector<Data> get_data_range(int start_idx, int count, std::function<bool()>&& compare_function) const {
			std::shared_lock lock(mutex_);

		}*/

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
