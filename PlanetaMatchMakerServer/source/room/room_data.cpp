#include "room_data.hpp"

namespace pgl {
	std::function<bool(const room_data&, const room_data&)> get_room_data_compare_function(
		const room_data_sort_kind sort_kind, const std::string& search_name) {
		std::function<bool(const room_data&, const room_data&)> default_comparer;
		switch (sort_kind) {
			case room_data_sort_kind::name_ascending:
				default_comparer = [](const room_data& left, const room_data& right) {
					return left.name < right.name;
				};
				break;
			case room_data_sort_kind::name_descending:
				default_comparer = [](const room_data& left, const room_data& right) {
					return left.name > right.name;
				};
				break;
			case room_data_sort_kind::create_datetime_ascending:
				default_comparer = [](const room_data& left, const room_data& right) {
					return left.create_datetime < right.create_datetime;
				};
				break;
			case room_data_sort_kind::create_datetime_descending:
				default_comparer = [](const room_data& left, const room_data& right) {
					return left.create_datetime > right.create_datetime;
				};
				break;
			default:
				throw std::runtime_error("Invalid room_data_sort_kind.");
		}

		if (search_name.length() == 0) {
			return default_comparer;
		}

		return [default_comparer=std::move(default_comparer), search_name](const room_data& left,
			const room_data& right) {
			if (left.name.to_string() == search_name) {
				return true;
			}

			if (right.name.to_string() == search_name) {
				return false;
			}

			return default_comparer(left, right);
		};
	}

	std::function<bool(const room_data&)> get_room_data_filter_function(room_search_target_flag search_target_flags,
		const std::string& search_name) {
		const auto setting_filter = [search_target_flags](const room_data& data) {
			auto search_source_flags = room_search_target_flag::none;
			search_source_flags |= (data.setting_flags & room_setting_flag::public_room) != room_setting_flag::none
										? room_search_target_flag::public_room
										: room_search_target_flag::private_room;
			search_source_flags |= (data.setting_flags & room_setting_flag::open_room) != room_setting_flag::none
										? room_search_target_flag::open_room
										: room_search_target_flag::closed_room;
			return (search_source_flags & search_target_flags) != room_search_target_flag::none;
		};

		if (search_name.length() == 0) {
			return setting_filter;
		}

		return [setting_filter = std::move(setting_filter), search_name](const room_data& data) {
			const auto room_name = data.name.to_string();
			return setting_filter(data) && room_name.find(search_name) != std::string::npos;
		};
	}

	std::ostream& operator<<(std::ostream& os, const room_data& room_data) {
		os << "room(ID=" << room_data.room_id << ", name=" << room_data.name << ")";
		return os;
	}

	std::ostream& operator<<(std::ostream& os, const room_group_data& room_group_data) {
		os << "room_group(name=" << room_group_data.name << ")";
		return os;
	}
}
