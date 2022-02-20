#include "list_room_request_message_handler.hpp"

#include "server/server_data.hpp"
#include "utilities/checked_static_cast.hpp"
#include "../message_parameter_validator.hpp"

using namespace std;
using namespace boost;

namespace pgl {
	void list_room_request_message_handler::handle_message(const list_room_request_message& message,
		const std::shared_ptr<message_handle_parameter> param) {

		const message_parameter_validator_with_reply<message_type::list_room_reply, list_room_reply_message>
			parameter_validator(param);

		// Check room group existence
		const auto& room_data_container = param->server_data.get_room_data_container();

		list_room_reply_message reply{};

		// Generate room data list to send
		std::vector<room_data> matched_data_list;
		try {
			matched_data_list = room_data_container.get_data(message.sort_kind, message.search_target_flags,
				message.search_full_name);
			log_with_endpoint(log_level::info, param->socket.remote_endpoint(), matched_data_list.size(),
				" rooms are matched in ", room_data_container.size(), " rooms.");
		}
		catch (out_of_range&) {
			reply_message_header header{
				message_type::list_room_reply,
				message_error_code::request_parameter_wrong
			};

			log_with_endpoint(log_level::error, param->socket.remote_endpoint(), "Indicated sort_kind \"",
				static_cast<underlying_type_t<room_data_sort_kind>>(message.sort_kind), "\" is invalid.");
			send(param, header, reply);
			return;
		}

		// Prepare reply header
		reply_message_header header{
			message_type::list_room_reply,
			message_error_code::ok
		};
		reply.total_room_count = range_checked_static_cast<uint16_t>(room_data_container.size());
		reply.matched_room_count = range_checked_static_cast<uint16_t>(matched_data_list.size());
		reply.reply_room_count = std::min(range_checked_static_cast<uint16_t>(
				reply.matched_room_count <= message.start_index ? 0 : reply.matched_room_count - message.start_index),
			message.count);
		log_with_endpoint(log_level::info, param->socket.remote_endpoint(), matched_data_list.size(),
			" rooms are replied from index ", message.start_index, ".");

		// Reply room data list separately
		const auto separation = (reply.reply_room_count + list_room_reply_room_info_count - 1) /
			list_room_reply_room_info_count;
		log_with_endpoint(log_level::info, param->socket.remote_endpoint(), "Start replying ",
			message_type::list_room_request, " message by ", separation, " messages.");
		for (auto i = 0; i < separation; ++i) {
			for (auto j = 0; j < list_room_reply_room_info_count; ++j) {
				const auto reply_data_index = list_room_reply_room_info_count * i + j;
				if (reply_data_index < reply.reply_room_count) {
					const auto matched_data_index = message.start_index + reply_data_index;
					reply.room_info_list[j] = list_room_reply_message::room_info{
						matched_data_list[matched_data_index].room_id,
						matched_data_list[matched_data_index].host_player_full_name,
						matched_data_list[matched_data_index].setting_flags,
						matched_data_list[matched_data_index].max_player_count,
						matched_data_list[matched_data_index].current_player_count,
						matched_data_list[matched_data_index].create_datetime,
						matched_data_list[matched_data_index].game_host_connection_establish_mode
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
			reply.reply_room_count = 0;
			reply.room_info_list = {};
			send(param, header, reply);
		}

		log_with_endpoint(log_level::info, param->socket.remote_endpoint(), "Finished replying ",
			message_type::list_room_request, " message by ", separation, " messages.");
	}
}
