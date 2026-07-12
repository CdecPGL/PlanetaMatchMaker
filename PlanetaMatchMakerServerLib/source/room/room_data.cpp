#include "room_data.hpp"

namespace pgl {
	std::function<bool(const room_data&, const room_data&)> get_room_data_compare_function(
		const room_data_sort_kind sort_kind, const player_full_name& search_full_name) {
		std::function<bool(const room_data&, const room_data&)> default_comparer;
		switch (sort_kind) {
			case room_data_sort_kind::name_ascending:
				default_comparer = [](const room_data& left, const room_data& right) {
					return left.host_player_full_name.name == right.host_player_full_name.name
						       ? left.host_player_full_name.tag < right.host_player_full_name.tag
						       : left.host_player_full_name.name < right.host_player_full_name.name;
				};
				break;
			case room_data_sort_kind::name_descending:
				default_comparer = [](const room_data& left, const room_data& right) {
					return left.host_player_full_name.name == right.host_player_full_name.name
						       ? left.host_player_full_name.tag > right.host_player_full_name.tag
						       : left.host_player_full_name.name > right.host_player_full_name.name;
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
				throw std::out_of_range("Invalid room_data_sort_kind.");
		}

		if (!search_full_name.is_name_assigned()) { return default_comparer; }

		// We bring room whose name matches search name exactly to top. We don't consider tag because rooms whose tag don't match search tag are filtered in filter function.
		return [default_comparer=std::move(default_comparer), search_full_name](const room_data& left,
			const room_data& right) {
			if (left.host_player_full_name.name == search_full_name.name) { return true; }
			if (right.host_player_full_name.name == search_full_name.name) { return false; }
			return default_comparer(left, right);
		};
	}

	std::function<bool(const room_data&)> get_room_data_filter_function(room_search_target_flag search_target_flags,
		const player_full_name& search_full_name) {
		std::function<bool(const room_data&)> setting_filter = [search_target_flags](const room_data& data) {
			const auto public_flags = (data.setting_flags & room_setting_flag::public_room) != room_setting_flag::none
				                          ? room_search_target_flag::public_room
				                          : room_search_target_flag::private_room;
			const auto open_flags = (data.setting_flags & room_setting_flag::open_room) != room_setting_flag::none
				                        ? room_search_target_flag::open_room
				                        : room_search_target_flag::closed_room;
			return (public_flags & search_target_flags) != room_search_target_flag::none
				&& (open_flags & search_target_flags) != room_search_target_flag::none;
		};

		if (search_full_name.is_name_assigned()) {
			setting_filter = [setting_filter = std::move(setting_filter), search_name=search_full_name.name
				](const room_data& data) {
					const auto room_host_player_name = data.host_player_full_name.name.to_string();
					return setting_filter(data)
						&& room_host_player_name.find(search_name.to_string()) != std::string::npos;
				};
		}

		if (search_full_name.is_tag_assigned()) {
			setting_filter = [setting_filter = std::move(setting_filter), search_tag=search_full_name.tag
				](const room_data& data) {
					const auto room_host_player_tag = data.host_player_full_name.tag;
					return setting_filter(data) && room_host_player_tag == search_tag;
				};
		}

		return setting_filter;
	}

	std::ostream& operator<<(std::ostream& os, const room_data& room_data) {
		os << "room(ID=" << room_data.room_id << ", host_player=" << room_data
		                                                             .host_player_full_name.generate_full_name() << ")";
		return os;
	}
}
