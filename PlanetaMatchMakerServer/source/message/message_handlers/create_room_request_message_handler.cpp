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
using namespace minimal_serializer;

namespace pgl {
	create_room_request_message_handler::handle_return_t create_room_request_message_handler::handle_message(
		const create_room_request_message& message,
		const std::shared_ptr<message_handle_parameter> param) {
		const message_parameter_validator parameter_validator(param);

		auto& room_data_container = param->server_data.get_room_data_container();

		// Check port number is valid.
		if (message.connection_establish_mode == game_host_connection_establish_mode::builtin) {
			parameter_validator.validate_port_number(message.port_number);
		}

		// Check max player count is valid.
		parameter_validator.validate_max_player_count(message.max_player_count);

		// Client which is already hosting room cannot create room newly.
		if (param->session_data.is_hosting_room()) {
			const auto error_message = generate_string("Failed to create new room with player\"",
				param->session_data.client_player_name().generate_full_name(),
				"\" because this client is already hosting room with id ", param->session_data.hosting_room_id(),
				".");
			throw client_error(client_error_code::client_already_hosting_room, false, error_message);
		}

		// Check if room count reached limit
		if (room_data_container.size() == param->server_setting.common.max_room_count) {
			const auto error_message = generate_string("Failed to create new room with player\"",
				param->session_data.client_player_name().generate_full_name(),
				"\" because room count reaches max.");
			throw client_error(client_error_code::room_count_exceeds_limit, false, error_message);
		}

		try {
			// Create requested room
			const auto host_endpoint = endpoint::make_from_boost_endpoint(param->socket.remote_endpoint());
			auto game_host_endpoint = host_endpoint;
			game_host_endpoint.port_number = message.port_number;
			const auto is_public = message.password.length() == 0;
			const room_data room_data{
				{}, // assign in room_data_container.assign_id_and_add(room_data)
				param->session_data.client_player_name(),
				(is_public ? room_setting_flag::public_room : room_setting_flag::none) |
				room_setting_flag::open_room,
				message.password,
				message.max_player_count,
				datetime::now(),
				host_endpoint,
				message.connection_establish_mode,
				game_host_endpoint,
				message.external_id,
				1
			};

			create_room_reply_message reply{
				room_data_container.assign_id_and_add(room_data)
			};

			log_with_endpoint(log_level::info, param->socket.remote_endpoint(), "New ",
				is_public ? "public" : "private", " room for player \"",
				param->session_data.client_player_name().generate_full_name(), "\" is created with id: ",
				reply.room_id);
			param->session_data.set_hosting_room_id(reply.room_id);

			// Reply to the client
			return {{reply}, false};
		}
		catch (const unique_variable_duplication_error&) {
			const auto error_message = generate_string("Failed to create new room with player\"",
				param->session_data.client_player_name().generate_full_name(),
				"\" because the name is duplicated. This is not expected behavior.");
			throw server_error(false, error_message);
		}
	}
}
