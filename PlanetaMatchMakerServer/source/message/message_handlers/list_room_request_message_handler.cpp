#include "list_room_request_message_handler.hpp"

#include "async/read_write.hpp"
#include "async/timer.hpp"
#include "server/server_data.hpp"
#include "server/server_error.hpp"
#include "utilities/string_utility.hpp"
#include "utilities/static_cast_with_assertion.hpp"

using namespace std;
using namespace boost;

namespace pgl {
	void list_room_request_message_handler::handle_message(const list_room_request_message& message,
	                                                       std::shared_ptr<message_handle_parameter> param) {
		reply_message_header header{
			message_type::list_room_reply,
			message_error_code::ok
		};

		if (param->server_data->is_valid_room_group_index(message.group_index)) {
			const auto extra_message = generate_string("Range of valid room group index is 0 to ",
			                                           param->server_data->room_group_count(), " but \"",
			                                           message.group_index, "\" is requested.");
			header.error_code = message_error_code::room_group_index_out_of_range;
			send(param, header, list_room_reply{});
			throw server_error(server_error_code::room_group_index_out_of_range, extra_message);
		}

		const auto& room_data_container = param->server_data->get_room_data_container(message.group_index);

		auto room_data_list = room_data_container.get_range_data(message.start_index,
		                                                         message.end_index - message.start_index + 1,
		                                                         message.sort_kind);
		list_room_reply reply{
			static_cast_with_range_assertion<uint8_t>(room_data_container.size()),
			static_cast_with_range_assertion<uint8_t>(room_data_list.size()),
			{},
			{}
		};

		const auto separation = static_cast_with_range_assertion<int>(
			(room_data_list.size() / list_room_reply_room_info_count - 1) /
			list_room_reply_room_info_count);
		for (auto i = 0; i < separation; ++i) {
			const auto send_room_count = i == separation - 1
				                             ? static_cast_with_range_assertion<uint8_t>(
					                             static_cast_with_range_assertion<int>(room_data_list.size()) -
					                             list_room_reply_room_info_count * i)
				                             : list_room_reply_room_info_count;
			reply.reply_room_start_index = static_cast_with_range_assertion<uint8_t>(
				message.start_index + i * separation);
			reply.reply_room_end_index = static_cast_with_range_assertion<uint8_t>(
				message.start_index + i * separation + send_room_count
				- 1);
			for (auto j = 0; j < list_room_reply_room_info_count; ++j) {
				const auto send_room_idx = i * separation + j;
				if (send_room_idx < send_room_count) {
					reply.room_info_list[j] = list_room_reply::room_info{
						room_data_list[send_room_idx].room_id,
						room_data_list[send_room_idx].name,
						room_data_list[send_room_idx].flags,
						room_data_list[send_room_idx].max_player_count,
						room_data_list[send_room_idx].current_player_count,
						room_data_list[send_room_idx].create_datetime,
					};
				} else {
					reply.room_info_list[j] = {};
				}
			}

			send(param, header, reply);
		}
	}
}
