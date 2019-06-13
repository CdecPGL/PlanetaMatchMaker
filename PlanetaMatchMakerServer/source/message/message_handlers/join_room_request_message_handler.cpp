#include "join_room_request_message_handler.hpp"
#include "utilities/string_utility.hpp"
#include "../message_handle_utilities.hpp"

namespace pgl {
	void join_room_request_message_handler::handle_message(const join_room_request_message& message,
	                                                       std::shared_ptr<message_handle_parameter> param) {
		join_room_reply_message reply{};

		check_remote_endpoint_existence<message_type::join_room_reply>(param, reply);

		check_room_group_existence<message_type::join_room_reply>(param, message.group_index, reply);
		const auto& room_data_container = param->server_data->get_room_data_container(message.group_index);

		check_room_existence<message_type::join_room_reply>(param, room_data_container, message.room_id, reply);
		const auto room_data = room_data_container.get_data(message.room_id);

		if (room_data.current_player_count >= room_data.max_player_count) {
			const reply_message_header header{
				message_type::join_room_reply,
				message_error_code::player_count_reaches_limit
			};
			send(param, header, reply);
			const auto extra_message = generate_string("Requested room \"", message.room_id, "\" in room group \"",
			                                           message.group_index, "\" is full of players (",
			                                           room_data.current_player_count, ").");
			throw server_error(server_error_code::room_player_is_full, extra_message);
		}

		const reply_message_header header{
			message_type::join_room_reply,
			message_error_code::ok
		};
		reply.host_address = room_data.host_address;
		send(param, header, reply);
	}
}
