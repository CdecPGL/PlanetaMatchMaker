#include "server/server_data.hpp"
#include "async/read_write.hpp"
#include "async/timer.hpp"
#include "message/messages.hpp"
#include "server/server_error.hpp"
#include "client/client_address.hpp"
#include "utilities/log.hpp"
#include "datetime/datetime.hpp"
#include "create_room_request_message_handler.hpp"

using namespace boost;

namespace pgl {

	void create_room_request_message_handler::handle_message(const create_room_request_message& message,
	                                                         std::shared_ptr<message_handle_parameter> param) {
		create_room_reply_message reply{};

		check_remote_endpoint_authority<message_type::create_room_reply>(param, reply);

		if (!param->server_data->is_valid_room_group_index(message.group_index)) {
			const auto extra_message = generate_string("Range of valid room group index is 0 to ",
			                                           param->server_data->room_group_count(), " but \"",
			                                           message.group_index, "\" is requested.");
			reply_message_header header{
				message_type::create_room_reply,
				message_error_code::room_group_index_out_of_range
			};
			send(param, header, reply);
			throw server_error(server_error_code::room_group_index_out_of_range, extra_message);
		}

		// Create room
		reply_message_header header{
			message_type::create_room_reply,
			message_error_code::ok
		};
		room_data room_data{
			{}, // assign in room_data_container.assign_id_and_add_data(room_data)
			message.name,
			message.flags,
			message.password,
			message.max_player_count,
			datetime::now(),
			client_address::make_from_endpoint(param->socket.remote_endpoint()),
			1
		};
		reply.room_id = param->server_data->get_room_data_container(message.group_index).assign_id_and_add_data(
			room_data);
		log_with_endpoint(log_level::info, param->socket.remote_endpoint(), "New room \"", room_data.name,
		                  "\" is created in group ", message.group_index, " with id: ", reply.room_id);
		send(param, header, reply);
	}
}
