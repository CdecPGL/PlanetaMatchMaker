#include "update_room_status_request_message_handler.hpp"
#include "utilities/string_utility.hpp"
#include "../message_handle_utilities.hpp"

namespace pgl {

	void update_room_status_request_message_handler::handle_message(const update_room_status_request_message& message,
	                                                                std::shared_ptr<message_handle_parameter> param) {
		check_remote_endpoint_authority<message_type::update_room_status_reply>(
			param, update_room_status_reply_message{});

		if (!param->server_data->is_valid_room_group_index(message.group_index)) {
			const reply_message_header header{
				message_type::update_room_status_reply,
				message_error_code::room_group_index_out_of_range
			};
			send(param, header, update_room_status_reply_message{});
			const auto extra_message = generate_string("Range of valid room group index is 0 to ",
			                                           param->server_data->room_group_count(), " but \"",
			                                           message.group_index, "\" is requested.");
			throw server_error(server_error_code::room_group_index_out_of_range, extra_message);
		}
		auto& room_data_container = param->server_data->get_room_data_container(message.group_index);

		if (!room_data_container.is_data_exist(message.room_id)) {
			const reply_message_header header{
				message_type::update_room_status_reply,
				message_error_code::room_does_not_exist
			};
			send(param, header, update_room_status_reply_message{});
			const auto extra_message = generate_string("Requested room name \"", message.room_id,
			                                           "\" does not exist in room group \"", message.group_index,
			                                           "\".");
			throw server_error(server_error_code::room_does_not_exist, extra_message);
		}
		auto room_data = room_data_container.get_data(message.room_id);

		if (room_data.host_address != client_address::make_from_endpoint(param->socket.remote_endpoint())) {
			const reply_message_header header{
				message_type::update_room_status_reply,
				message_error_code::permission_denied
			};
			send(param, header, update_room_status_reply_message{});
			const auto extra_message = generate_string("The client is not host of requested room.");
			throw server_error(server_error_code::room_permission_error, extra_message);
		}

		switch (message.status) {
		case update_room_status_request_message::status::open:
			room_data.flags |= room_flags_bit_mask::is_open;
			room_data_container.update_data(room_data);
			break;
		case update_room_status_request_message::status::close:
			room_data.flags &= ~room_flags_bit_mask::is_open;
			room_data_container.update_data(room_data);
			break;
		case update_room_status_request_message::status::remove:
			room_data_container.remove_data(room_data.room_id);
			break;
		default: ;
		}

		const reply_message_header header{
			message_type::update_room_status_reply,
			message_error_code::ok
		};
		update_room_status_reply_message reply{};
		send(param, header, reply);
	}
}
