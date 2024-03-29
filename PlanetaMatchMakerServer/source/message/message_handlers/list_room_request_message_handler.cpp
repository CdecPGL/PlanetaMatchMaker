#include "list_room_request_message_handler.hpp"

#include "server/server_data.hpp"
#include "utilities/checked_static_cast.hpp"
#include "../message_parameter_validator.hpp"

using namespace std;
using namespace boost;
using namespace minimal_serializer;

namespace pgl {
	list_room_request_message_handler::handle_return_t list_room_request_message_handler::handle_message(
		const list_room_request_message& message,
		const std::shared_ptr<message_handle_parameter> param) {
		const message_parameter_validator parameter_validator(param);

		// Check room group existence
		const auto& room_data_container = param->server_data.get_room_data_container();

		// Generate room data list to send
		std::vector<room_data> matched_data_list;
		try {
			matched_data_list = room_data_container.search(message.sort_kind, message.search_target_flags,
				message.search_full_name);
			log_with_endpoint(log_level::info, param->socket.remote_endpoint(), matched_data_list.size(),
				" rooms are matched in ", room_data_container.size(), " rooms.");
		}
		catch (out_of_range&) {
			const auto error_message = generate_string("Indicated sort_kind \"",
				static_cast<underlying_type_t<room_data_sort_kind>>(message.sort_kind), "\" is invalid.");
			throw client_error(client_error_code::request_parameter_wrong, false, error_message);
		}

		// Prepare reply header
		list_room_reply_message reply{};
		reply.total_room_count = range_checked_static_cast<uint16_t>(room_data_container.size());
		reply.matched_room_count = range_checked_static_cast<uint16_t>(matched_data_list.size());
		reply.reply_room_count = std::min(range_checked_static_cast<uint16_t>(
				reply.matched_room_count <= message.start_index ? 0 : reply.matched_room_count - message.start_index),
			message.count);
		log_with_endpoint(log_level::info, param->socket.remote_endpoint(), matched_data_list.size(),
			" rooms are replied from index ", message.start_index, ".");

		// Generate reply bodies separately
		const auto separation = (reply.reply_room_count + list_room_reply_room_info_count - 1) /
			list_room_reply_room_info_count;
		std::vector<list_room_reply_message> reply_bodies;
		for (auto i = 0; i < separation; ++i) {
			for (auto j = 0; j < list_room_reply_room_info_count; ++j) {
				if (const auto reply_data_index = list_room_reply_room_info_count * i + j; reply_data_index < reply.
					reply_room_count) {
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

			log_with_endpoint(log_level::debug, param->socket.remote_endpoint(), "Generate reply body ", i + 1, "/",
				separation,
				" message.");
			reply_bodies.push_back(reply);
		}

		if (separation == 0) {
			log_with_endpoint(log_level::debug, param->socket.remote_endpoint(),
				"There are no room which matches request.");
			reply.reply_room_count = 0;
			reply.room_info_list = {};
			reply_bodies.push_back(reply);
		}

		log_with_endpoint(log_level::info, param->socket.remote_endpoint(), "Finished generating reply bodies ",
			message_type::list_room, " message by ", separation, " messages.");

		return {reply_bodies, false};
	}
}
