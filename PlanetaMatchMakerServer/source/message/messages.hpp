#pragma once

#include <cstdint>

#include "minimal_serializer/serializer.hpp"
#include "message/message_error_code.hpp"
#include "datetime/datetime.hpp"
#include "network/endpoint.hpp"
#include "room/room_constants.hpp"
#include "data/data_constants.hpp"
#include "room/room_data.hpp"
#include "session/session_key.hpp"
#include "client/client_constants.hpp"
#include "message_constants.hpp"

namespace pgl {
	enum class message_type : uint8_t {
		authentication_request,
		authentication_reply,
		create_room_request,
		create_room_reply,
		list_room_request,
		list_room_reply,
		join_room_request,
		join_room_reply,
		update_room_status_notice,
		connection_test_request,
		connection_test_reply,
		random_match_request,
		keep_alive_notice
	};

	// 5 bytes. Use for notice message too
	struct request_message_header final {
		message_type message_type;
		session_key_type session_key;

		void on_serialize(minimal_serializer::serializer& serializer) {
			serializer += message_type;
			serializer += session_key;
		}
	};

	// 2 bytes
	struct reply_message_header final {
		message_type message_type;
		message_error_code error_code;

		void on_serialize(minimal_serializer::serializer& serializer) {
			serializer += message_type;
			serializer += error_code;
		}
	};

	// size of message should be less than (256 bytes - header size)

	// 26 bytes
	struct authentication_request_message final {
		version_type version;
		player_name_t player_name;

		void on_serialize(minimal_serializer::serializer& serializer) {
			serializer += version;
			serializer += player_name;
		}
	};

	// 8 bytes
	struct authentication_reply_message final {
		version_type version;
		session_key_type session_key;
		player_tag_t player_tag;

		void on_serialize(minimal_serializer::serializer& serializer) {
			serializer += version;
			serializer += session_key;
			serializer += player_tag;
		}
	};

	// 19 bytes
	struct create_room_request_message final {
		room_password_t password;
		uint8_t max_player_count;
		port_number_type port_number;

		void on_serialize(minimal_serializer::serializer& serializer) {
			serializer += password;
			serializer += max_player_count;
			serializer += port_number;
		}
	};

	// 4 bytes
	struct create_room_reply_message final {
		room_id_t room_id;

		void on_serialize(minimal_serializer::serializer& serializer) { serializer += room_id; }
	};

	// 32 bytes
	struct list_room_request_message final {
		uint16_t start_index;
		uint16_t count;
		room_data_sort_kind sort_kind;
		room_search_target_flag search_target_flags;
		player_full_name search_full_name;

		void on_serialize(minimal_serializer::serializer& serializer) {
			serializer += start_index;
			serializer += count;
			serializer += sort_kind;
			serializer += search_target_flags;
			serializer += search_full_name;
		}
	};

	// 252 bytes
	struct list_room_reply_message final {
		//41 bytes
		struct room_info final {
			room_id_t room_id;
			player_full_name host_player_full_name;
			room_setting_flag setting_flags;
			uint8_t max_player_count;
			uint8_t current_player_count;
			datetime create_datetime;

			void on_serialize(minimal_serializer::serializer& serializer) {
				serializer += room_id;
				serializer += host_player_full_name;
				serializer += setting_flags;
				serializer += max_player_count;
				serializer += current_player_count;
				serializer += create_datetime;
			}
		};

		uint16_t total_room_count; // the number of rooms server managing
		uint16_t matched_room_count; // the number of rooms matched to requested condition
		uint16_t reply_room_count; // the number of rooms in these replies
		std::array<room_info, list_room_reply_room_info_count> room_info_list;

		void on_serialize(minimal_serializer::serializer& serializer) {
			serializer += total_room_count;
			serializer += matched_room_count;
			serializer += reply_room_count;
			serializer += room_info_list;
		}
	};

	// 20 bytes
	struct join_room_request_message final {
		room_id_t room_id;
		room_password_t password;

		void on_serialize(minimal_serializer::serializer& serializer) {
			serializer += room_id;
			serializer += password;
		}
	};

	//18 bytes
	struct join_room_reply_message final {
		endpoint game_host_endpoint;

		void on_serialize(minimal_serializer::serializer& serializer) { serializer += game_host_endpoint; }
	};

	// 7 bytes
	struct update_room_status_notice_message final {
		enum class status : uint8_t { open, close, remove };

		room_id_t room_id;
		status status;
		bool is_current_player_count_changed;
		uint8_t current_player_count;

		void on_serialize(minimal_serializer::serializer& serializer) {
			serializer += room_id;
			serializer += status;
			serializer += is_current_player_count_changed;
			serializer += current_player_count;
		}
	};

	// 3 bytes
	struct connection_test_request_message final {
		transport_protocol protocol;
		port_number_type port_number;

		void on_serialize(minimal_serializer::serializer& serializer) {
			serializer += protocol;
			serializer += port_number;
		}
	};

	// 1 bytes
	struct connection_test_reply_message final {
		bool succeed;

		void on_serialize(minimal_serializer::serializer& serializer) { serializer += succeed; }
	};

	struct random_match_request_message final {
		void on_serialize(minimal_serializer::serializer& serializer [[maybe_unused]]) { }
	};

	// 1 byte
	struct keep_alive_notice_message final {
		uint8_t dummy;

		void on_serialize(minimal_serializer::serializer& serializer) { serializer += dummy; }
	};
}
