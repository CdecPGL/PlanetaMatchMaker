#include "list_room_request_message_handler.hpp"

#include "server/server_data.hpp"
#include "utilities/checked_static_cast.hpp"
#include "../message_handle_utilities.hpp"

using namespace std;
using namespace boost;

namespace pgl {
	void list_room_request_message_handler::handle_message(const list_room_request_message& message,
		std::shared_ptr<message_handle_parameter> param) {

		list_room_reply_message reply{};

		// Check room group existence
		check_room_group_existence<message_type::list_room_reply>(param, message.group_index, reply);
		const auto& room_data_container = param->server_data.get_room_data_container(message.group_index);

		// Generate room data list to send
		auto room_data_list = room_data_container.get_range_data(message.start_index,
			message.end_index - message.start_index + 1,
			message.sort_kind, message.search_target_flags, message.search_name.to_string());
		log_with_endpoint(log_level::info, param->socket.remote_endpoint(), room_data_list.size(),
			" rooms are found from ", room_data_container.size(), " rooms.");

		// Prepare reply header
		reply_message_header header{
			message_type::list_room_reply,
			message_error_code::ok
		};
		reply.total_room_count = range_checked_static_cast<uint8_t>(room_data_container.size());
		reply.result_room_count = range_checked_static_cast<uint8_t>(room_data_list.size());

		// Reply room data list separately
		const auto separation = range_checked_static_cast<int>(
			(room_data_list.size() + list_room_reply_room_info_count - 1) / list_room_reply_room_info_count);
		log_with_endpoint(log_level::info, param->socket.remote_endpoint(), "Start replying ",
			message_type::list_room_request, " message by ", separation, " messages.");
		for (auto i = 0; i < separation; ++i) {
			const auto send_room_count = i == separation - 1
				                             ? range_checked_static_cast<uint8_t>(
					                             range_checked_static_cast<int>(room_data_list.size()) -
					                             list_room_reply_room_info_count * i)
				                             : list_room_reply_room_info_count;
			reply.reply_room_start_index = range_checked_static_cast<uint8_t>(
				message.start_index + i * separation);
			reply.reply_room_count = range_checked_static_cast<uint8_t>(send_room_count);
			for (auto j = 0; j < list_room_reply_room_info_count; ++j) {
				const auto send_room_idx = i * separation + j;
				if (send_room_idx < send_room_count) {
					reply.room_info_list[j] = list_room_reply_message::room_info{
						room_data_list[send_room_idx].room_id,
						room_data_list[send_room_idx].name,
						room_data_list[send_room_idx].setting_flags,
						room_data_list[send_room_idx].max_player_count,
						room_data_list[send_room_idx].current_player_count,
						room_data_list[send_room_idx].create_datetime,
					};
				}
				else { reply.room_info_list[j] = {}; }
			}

			log_with_endpoint(log_level::debug, param->socket.remote_endpoint(), "Reply ", i + 1, "/", separation,
				" message.");
			send(param, header, reply);
		}

		if (separation == 0) {
			log_with_endpoint(log_level::debug, param->socket.remote_endpoint(),
				"There are no room which matches request.");
			reply.reply_room_start_index = 0;
			reply.reply_room_count = 0;
			reply.room_info_list = {};
			send(param, header, reply);
		}

		log_with_endpoint(log_level::info, param->socket.remote_endpoint(), "Finished replying ",
			message_type::list_room_request, " message by ", separation, " messages.");
	}
}
