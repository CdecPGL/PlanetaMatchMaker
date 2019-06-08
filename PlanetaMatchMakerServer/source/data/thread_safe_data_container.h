#pragma once

#include <unordered_map>
#include <atomic>
#include <shared_mutex>

#include <boost/noncopyable.hpp>
#include <boost/call_traits.hpp>

namespace pgl {
	// A thread safe container of data
	template <typename TId, typename TData>
	class thread_safe_data_container final : boost::noncopyable {
	public:
		using id_param_type = typename boost::call_traits<TId>::param_type;
		using data_param_type = typename boost::call_traits<TData>::param_type;

		[[nodiscard]] bool is_data_exist(id_param_type id) const {
			std::shared_lock<decltype(mutex_)> lock(mutex_);
			return data_map_.find(id) != data_map_.end();
		}

		[[nodiscard]] TData get_data(id_param_type id) const {
			std::shared_lock<decltype(mutex_)> lock(mutex_);
			return data_map_.at(id).load();
		}

		void add_data(id_param_type id, data_param_type data) {
			std::lock_guard<decltype(mutex_)> lock(mutex_);
			data_map_.emplace(id, data);
		}

		void update_data(id_param_type id, data_param_type data) {
			std::shared_lock<decltype(mutex_)> lock(mutex_);
			data_map_.at(id) = data;
		}

	private:
		std::unordered_map<TId, std::atomic<TData>> data_map_;
		mutable std::shared_mutex mutex_;
	};
}
