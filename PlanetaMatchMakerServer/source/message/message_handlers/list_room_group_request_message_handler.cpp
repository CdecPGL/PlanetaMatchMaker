#include "nameof.hpp"

#include "server/server_data.hpp"
#include "async/timer.hpp"
#include "async/read_write.hpp"
#include "server/server_error.hpp"
#include "utilities/log.hpp"
#include "utilities/static_cast_with_assertion.hpp"
#include "list_room_group_request_message_handler.hpp"

using namespace std;
using namespace boost;

namespace pgl {
	void list_room_group_request_message_handler::handle_message(const list_room_group_request_message& message,
	                                                             message_handle_parameter& param) {
		reply_message_header header{
			message_type::list_room_group_reply,
			message_error_code::ok
		};

		const auto& room_group_data_list = param.server_data->get_room_data_group_list();
		decltype(list_room_group_reply_message::room_group_info_list) room_group_info_list;
		std::transform(room_group_data_list.begin(), room_group_data_list.end(), room_group_info_list.begin(),
		               [](const room_group_data& data)
		               {
			               return list_room_group_reply_message::room_group_info{data.name};
		               });
		list_room_group_reply_message reply{
			static_cast_with_range_assertion<uint8_t>(room_group_data_list.size()),
			std::move(room_group_info_list)
		};

		try {
			execute_timed_async_operation(param.io_service, param.socket, param.timeout_seconds, [&]()
			{
				packed_async_write(param.socket, param.yield, header, reply);
			});
			log_with_endpoint(log_level::info, param.socket.remote_endpoint(), "Reply ",
			                  NAMEOF_ENUM(message_type::list_room_group_request), " message.");
		} catch (const system::system_error& e) {
			throw server_error(server_error_code::message_send_error, e.code().message());
		}
	}
}
