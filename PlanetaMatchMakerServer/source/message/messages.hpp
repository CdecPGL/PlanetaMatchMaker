#pragma once

#include <cstdint>

#include "minimal_serializer/serializer.hpp"
#include "message/message_error_code.hpp"
#include "datetime/datetime.hpp"
#include "network/endpoint.hpp"
#include "room/room_constants.hpp"
#include "data/data_constants.hpp"
#include "room/room_data.hpp"
#include "client/client_constants.hpp"
#include "authentication/game.hpp"
#include "message_constants.hpp"
#include "authentication/authentication_result.hpp"

namespace pgl {
	enum class message_type : uint8_t {
		authentication,
		create_room,
		list_room,
		join_room,
		update_room_status,
		connection_test,
		random_match,
		keep_alive
	};

	// 1 bytes. Use for notice message too
	struct request_message_header final {
		message_type message_type;

		using serialize_targets = minimal_serializer::serialize_target_container<
			&request_message_header::message_type
		>;
	};

	// 2 bytes
	struct reply_message_header final {
		message_type message_type;
		message_error_code error_code;

		using serialize_targets = minimal_serializer::serialize_target_container<
			&reply_message_header::message_type,
			&reply_message_header::error_code
		>;
	};

	// size of message should be less than (256 bytes - header size)

	// 74 bytes
	struct authentication_request_message final {
		api_version_type api_version;
		game_id_t game_id;
		game_version_t game_version;
		player_name_t player_name;

		using serialize_targets = minimal_serializer::serialize_target_container<
			&authentication_request_message::api_version,
			&authentication_request_message::game_id,
			&authentication_request_message::game_version,
			&authentication_request_message::player_name
		>;
	};

	// 29 bytes
	struct authentication_reply_message final {
		authentication_result result;
		api_version_type api_version;
		game_version_t game_version;
		player_tag_t player_tag;

		using serialize_targets = minimal_serializer::serialize_target_container<
			&authentication_reply_message::api_version,
			&authentication_reply_message::game_version,
			&authentication_reply_message::player_tag
		>;
	};

	// 84 bytes
	struct create_room_request_message final {
		room_password_t password;
		uint8_t max_player_count;
		game_host_connection_establish_mode connection_establish_mode;
		port_number_type port_number;
		game_host_external_id_t external_id;

		using serialize_targets = minimal_serializer::serialize_target_container<
			&create_room_request_message::password,
			&create_room_request_message::max_player_count,
			&create_room_request_message::connection_establish_mode,
			&create_room_request_message::port_number,
			&create_room_request_message::external_id
		>;
	};

	// 4 bytes
	struct create_room_reply_message final {
		room_id_t room_id;

		using serialize_targets = minimal_serializer::serialize_target_container<
			&create_room_reply_message::room_id
		>;
	};

	// 30 bytes
	struct list_room_request_message final {
		uint16_t start_index;
		uint16_t count;
		room_data_sort_kind sort_kind;
		room_search_target_flag search_target_flags;
		player_full_name search_full_name;

		using serialize_targets = minimal_serializer::serialize_target_container<
			&list_room_request_message::start_index,
			&list_room_request_message::count,
			&list_room_request_message::sort_kind,
			&list_room_request_message::search_target_flags,
			&list_room_request_message::search_full_name
		>;
	};

	// 246 bytes
	struct list_room_reply_message final {
		//40 bytes
		struct room_info final {
			room_id_t room_id;
			player_full_name host_player_full_name;
			room_setting_flag setting_flags;
			uint8_t max_player_count;
			uint8_t current_player_count;
			datetime create_datetime;
			game_host_connection_establish_mode connection_establish_mode;

			using serialize_targets = minimal_serializer::serialize_target_container<
				&room_info::room_id,
				&room_info::host_player_full_name,
				&room_info::setting_flags,
				&room_info::max_player_count,
				&room_info::current_player_count,
				&room_info::create_datetime,
				&room_info::connection_establish_mode
			>;
		};

		uint16_t total_room_count; // the number of rooms server managing
		uint16_t matched_room_count; // the number of rooms matched to requested condition
		uint16_t reply_room_count; // the number of rooms in these replies
		std::array<room_info, list_room_reply_room_info_count> room_info_list;

		using serialize_targets = minimal_serializer::serialize_target_container<
			&list_room_reply_message::total_room_count,
			&list_room_reply_message::matched_room_count,
			&list_room_reply_message::reply_room_count,
			&list_room_reply_message::room_info_list
		>;
	};

	// 21 bytes
	struct join_room_request_message final {
		room_id_t room_id;
		game_host_connection_establish_mode connection_establish_mode;
		room_password_t password;

		using serialize_targets = minimal_serializer::serialize_target_container<
			&join_room_request_message::room_id,
			&join_room_request_message::connection_establish_mode,
			&join_room_request_message::password
		>;
	};

	// 82 bytes
	struct join_room_reply_message final {
		endpoint game_host_endpoint;
		game_host_external_id_t game_host_external_id;

		using serialize_targets = minimal_serializer::serialize_target_container<
			&join_room_reply_message::game_host_endpoint,
			&join_room_reply_message::game_host_external_id
		>;
	};

	// 7 bytes
	struct update_room_status_notice_message final {
		enum class status : uint8_t { open, close, remove };

		room_id_t room_id;
		status status;
		bool is_current_player_count_changed;
		uint8_t current_player_count;

		using serialize_targets = minimal_serializer::serialize_target_container<
			&update_room_status_notice_message::room_id,
			&update_room_status_notice_message::status,
			&update_room_status_notice_message::is_current_player_count_changed,
			&update_room_status_notice_message::current_player_count
		>;
	};

	// 3 bytes
	struct connection_test_request_message final {
		transport_protocol protocol;
		port_number_type port_number;

		using serialize_targets = minimal_serializer::serialize_target_container<
			&connection_test_request_message::protocol,
			&connection_test_request_message::port_number
		>;
	};

	// 1 bytes
	struct connection_test_reply_message final {
		bool succeed;

		using serialize_targets = minimal_serializer::serialize_target_container<
			&connection_test_reply_message::succeed
		>;
	};

	struct random_match_request_message final {
		uint8_t dummy;

		using serialize_targets = minimal_serializer::serialize_target_container<
			&random_match_request_message::dummy
		>;
	};

	// 1 byte
	struct keep_alive_notice_message final {
		uint8_t dummy;

		using serialize_targets = minimal_serializer::serialize_target_container<
			&keep_alive_notice_message::dummy
		>;
	};
}
