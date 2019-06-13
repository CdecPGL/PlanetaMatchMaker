#include "nameof.hpp"

#include "server/server_data.hpp"
#include "async/timer.hpp"
#include "utilities/log.hpp"
#include "utilities/static_cast_with_assertion.hpp"
#include "list_room_group_request_message_handler.hpp"
#include "../message_handle_utilities.hpp"

using namespace std;
using namespace boost;

namespace pgl {
	void list_room_group_request_message_handler::handle_message(const list_room_group_request_message& message,
	                                                             std::shared_ptr<message_handle_parameter> param) {
		list_room_group_reply_message reply{};

		check_remote_endpoint_existence<message_type::list_room_group_reply>(param, reply);

		const auto& room_group_data_list = param->server_data->get_room_data_group_list();
		decltype(list_room_group_reply_message::room_group_info_list) room_group_info_list;
		std::transform(room_group_data_list.begin(), room_group_data_list.end(), room_group_info_list.begin(),
		               [](const room_group_data& data)
		               {
			               return list_room_group_reply_message::room_group_info{data.name};
		               });

		const reply_message_header header{
			message_type::list_room_group_reply,
			message_error_code::ok
		};
		reply.room_group_count = static_cast_with_range_assertion<uint8_t>(room_group_data_list.size());
		reply.room_group_info_list = std::move(room_group_info_list);
		send(param, header, reply);
		log_with_endpoint(log_level::info, param->socket.remote_endpoint(), "Reply ",
		                  message_type::list_room_group_request, " message.");
	}
}
