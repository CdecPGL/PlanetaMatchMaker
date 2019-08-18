#include "server/server_data.hpp"
#include "async/read_write.hpp"
#include "async/timer.hpp"
#include "message/messages.hpp"
#include "client/client_address.hpp"
#include "utilities/log.hpp"
#include "datetime/datetime.hpp"
#include "session/session_data.hpp"
#include "create_room_request_message_handler.hpp"
#include "../message_handle_utilities.hpp"

using namespace boost;

namespace pgl {
	void create_room_request_message_handler::handle_message(const create_room_request_message& message,
		std::shared_ptr<message_handle_parameter> param) {

		create_room_reply_message reply{};

		// Check room group existence
		check_room_group_existence<message_type::create_room_reply>(param, message.group_index, reply);
		auto& room_data_container = param->server_data.get_room_data_container(message.group_index);

		// Create requested room
		reply_message_header header{
			message_type::create_room_reply,
			message_error_code::ok
		};
		room_data room_data{
			{}, // assign in room_data_container.assign_id_and_add_data(room_data)
			message.name,
			(message.is_public ? room_setting_flag::public_room : room_setting_flag::none) | room_setting_flag::
			open_room,
			message.password,
			message.max_player_count,
			datetime::now(),
			client_address::make_from_endpoint(param->socket.remote_endpoint()),
			1
		};
		try {
			reply.room_id = room_data_container.assign_id_and_add_data(room_data);
			log_with_endpoint(log_level::info, param->socket.remote_endpoint(), "New room \"", room_data.name,
				"\" is created in group ", message.group_index, " with id: ", reply.room_id);
			param->session_data.set_hosting_room_id(message.group_index, reply.room_id);

			// Reply to the client
			log_with_endpoint(log_level::info, param->socket.remote_endpoint(), "Reply ",
				message_type::create_room_request,
				" message.");
			send(param, header, reply);
		} catch (const unique_variable_duplication_error&) {
			log_with_endpoint(log_level::info, param->socket.remote_endpoint(), "Failed to create new room \"",
				room_data.name, "\" because the name is duplicated");
			header.error_code = message_error_code::room_name_duplicated;
			send(param, header, reply);
		}
	}
}
