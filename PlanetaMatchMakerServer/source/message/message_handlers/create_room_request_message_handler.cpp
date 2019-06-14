#include "server/server_data.hpp"
#include "async/read_write.hpp"
#include "async/timer.hpp"
#include "message/messages.hpp"
#include "client/client_address.hpp"
#include "utilities/log.hpp"
#include "datetime/datetime.hpp"
#include "create_room_request_message_handler.hpp"
#include "../message_handle_utilities.hpp"

using namespace boost;

namespace pgl {
	void create_room_request_message_handler::handle_message(const create_room_request_message& message,
		std::shared_ptr<message_handle_parameter> param) {
		create_room_reply_message reply{};

		// Check authority of the client
		check_remote_endpoint_existence<message_type::create_room_reply>(param, reply);

		// Check room group existence
		check_room_group_existence<message_type::create_room_reply>(param, message.group_index, reply);
		auto& room_data_container = param->server_data->get_room_data_container(message.group_index);

		// Create requested room
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
		reply.room_id = room_data_container.assign_id_and_add_data(room_data);
		log_with_endpoint(log_level::info, param->socket.remote_endpoint(), "New room \"", room_data.name,
			"\" is created in group ", message.group_index, " with id: ", reply.room_id);

		// Reply to the client
		send(param, header, reply);
	}
}
