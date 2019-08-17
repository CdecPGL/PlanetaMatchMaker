#include "nameof.hpp"

#include "server/server_data.hpp"
#include "server/server_setting.hpp"
#include "async/timer.hpp"
#include "utilities/log.hpp"
#include "utilities/checked_static_cast.hpp"
#include "list_room_group_request_message_handler.hpp"
#include "../message_handle_utilities.hpp"

using namespace std;
using namespace boost;

namespace pgl {
	void list_room_group_request_message_handler::handle_message(const list_room_group_request_message&,
		std::shared_ptr<message_handle_parameter> param) {

		list_room_group_reply_message reply{};

		// Generate room group data list to send
		const auto& room_group_data_list = param->server_data.get_room_data_group_list();
		decltype(list_room_group_reply_message::room_group_info_list) room_group_info_list;
		std::transform(room_group_data_list.begin(), room_group_data_list.end(), room_group_info_list.begin(),
			[](const room_group_data& data) {
				return list_room_group_reply_message::room_group_info{data.name};
			});
		log_with_endpoint(log_level::info, param->socket.remote_endpoint(), room_group_data_list.size(), "are listed.");

		// Reply to the client
		const reply_message_header header{
			message_type::list_room_group_reply,
			message_error_code::ok
		};
		reply.room_group_count = range_checked_static_cast<uint8_t>(room_group_data_list.size());
		reply.max_room_count_per_room_group = param->server_setting.max_room_per_room_group;
		reply.room_group_info_list = std::move(room_group_info_list);
		log_with_endpoint(log_level::info, param->socket.remote_endpoint(), "Reply ",
			message_type::list_room_group_request, " message.");
		send(param, header, reply);
	}
}
