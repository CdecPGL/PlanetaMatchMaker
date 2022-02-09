#include "server/server_data.hpp"
#include "server/server_setting.hpp"
#include "async/read_write.hpp"
#include "message/messages.hpp"
#include "network/endpoint.hpp"
#include "logger/log.hpp"
#include "datetime/datetime.hpp"
#include "session/session_data.hpp"
#include "create_room_request_message_handler.hpp"
#include "../message_parameter_validator.hpp"

using namespace boost;

namespace pgl {
	void create_room_request_message_handler::handle_message(const create_room_request_message& message,
		const std::shared_ptr<message_handle_parameter> param) {

		const message_parameter_validator_with_reply<message_type::create_room_reply, create_room_reply_message>
			parameter_validator(param);

		auto& room_data_container = param->server_data.get_room_data_container();

		// Check port number is valid
		parameter_validator.validate_port_number(message.port_number);

		// Check max player count is valid.
		parameter_validator.validate_max_player_count(message.max_player_count);

		reply_message_header header{
			message_type::create_room_reply,
			message_error_code::ok
		};

		create_room_reply_message reply{};

		// Client which is already hosting room cannot create room newly.
		if (param->session_data.is_hosting_room()) {
			log_with_endpoint(log_level::error, param->socket.remote_endpoint(),
				"Failed to create new room with player\"",
				param->session_data.client_player_name().generate_full_name(),
				"\" because this client is already hosting room with id ", param->session_data.hosting_room_id(),
				".");
			header.error_code = message_error_code::client_already_hosting_room;
			send(param, header, reply);
			return;
		}

		// Check if room count reached limit in room group
		if (room_data_container.size() == param->server_setting.common.max_room_count) {
			log_with_endpoint(log_level::error, param->socket.remote_endpoint(),
				"Failed to create new room with player\"",
				param->session_data.client_player_name().generate_full_name(),
				"\" because room count reaches max.");
			header.error_code = message_error_code::room_group_full;
			send(param, header, reply);
			return;
		}

		try {
			// Create requested room
			const auto host_endpoint = endpoint::make_from_boost_endpoint(param->socket.remote_endpoint());
			auto game_host_endpoint = host_endpoint;
			game_host_endpoint.port_number = message.port_number;
			const auto is_public = message.password.length() == 0;
			room_data room_data{
				{}, // assign in room_data_container.assign_id_and_add_data(room_data)
				param->session_data.client_player_name(),
				(is_public ? room_setting_flag::public_room : room_setting_flag::none) |
				room_setting_flag::open_room,
				message.password,
				message.max_player_count,
				datetime::now(),
				host_endpoint,
				game_host_endpoint,
				1
			};

			reply.room_id = room_data_container.assign_id_and_add_data(room_data);
			log_with_endpoint(log_level::info, param->socket.remote_endpoint(), "New ",
				is_public ? "public" : "private", " room for player \"",
				param->session_data.client_player_name().generate_full_name(), "\" is created with id: ",
				reply.room_id);
			param->session_data.set_hosting_room_id(reply.room_id);

			// Reply to the client
			log_with_endpoint(log_level::info, param->socket.remote_endpoint(), "Reply ",
				message_type::create_room_request,
				" message.");
			send(param, header, reply);
		}
		catch (const unique_variable_duplication_error&) {
			log_with_endpoint(log_level::error, param->socket.remote_endpoint(),
				"Failed to create new room with player\"",
				param->session_data.client_player_name().generate_full_name(),
				"\" because the name is duplicated. This is not expected behavior.");
			header.error_code = message_error_code::server_error;
			send(param, header, reply);
		}
	}
}
