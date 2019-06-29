#pragma once

#include <cstdint>

#include "serialize/serializer.hpp"
#include "message/message_error_code.hpp"
#include "datetime/datetime.hpp"
#include "client/client_address.hpp"
#include "room/room_constants.hpp"
#include "data/data_constants.hpp"
#include "room/room_data.hpp"
#include "message_constants.hpp"

namespace pgl {
	enum class message_type : uint8_t {
		authentication_request,
		authentication_reply,
		list_room_group_request,
		list_room_group_reply,
		create_room_request,
		create_room_reply,
		list_room_request,
		list_room_reply,
		join_room_request,
		join_room_reply,
		update_room_status_notice,
		random_match_request
	};

	// 1 bytes. Use for notice message too
	struct request_message_header final {
		message_type message_type{};

		void on_serialize(serializer& serializer) {
			serializer += message_type;
		}
	};

	// 2 bytes
	struct reply_message_header final {
		message_type message_type{};
		message_error_code error_code{};

		void on_serialize(serializer& serializer) {
			serializer += message_type;
			serializer += error_code;
		}
	};

	// size of message should be less than (256 bytes - header size)

	// 2 bytes
	struct authentication_request_message final {
		version_type version{};

		void on_serialize(serializer& serializer) {
			serializer += version;
		}
	};

	// 2 bytes
	struct authentication_reply_message final {
		version_type version{};

		void on_serialize(serializer& serializer) {
			serializer += version;
		}
	};

	// 1 bytes
	struct list_room_group_request_message final {
		uint8_t dummy;

		void on_serialize(serializer& serializer) {
			serializer += dummy;
		}
	};

	// 241 bytes
	struct list_room_group_reply_message final {
		struct room_group_info final {
			room_group_name_type name;

			void on_serialize(serializer& serializer) {
				serializer += name;
			}
		};

		uint8_t room_group_count{};
		std::array<room_group_info, room_group_max_count> room_group_info_list{};

		void on_serialize(serializer& serializer) {
			serializer += room_group_count;
			serializer += room_group_info_list;
		}
	};

	// 43 bytes
	struct create_room_request_message final {
		uint8_t group_index{};
		room_name_type name{};
		room_flags_bit_mask::flags_type flags{};
		room_password_type password{};
		uint8_t max_player_count{};

		void on_serialize(serializer& serializer) {
			serializer += group_index;
			serializer += name;
			serializer += flags;
			serializer += password;
			serializer += max_player_count;
		}
	};

	// 4 bytes
	struct create_room_reply_message final {
		room_id_type room_id{};

		void on_serialize(serializer& serializer) {
			serializer += room_id;
		}
	};

	// 5 bytes
	struct list_room_request_message final {
		uint8_t group_index{};
		uint8_t start_index{};
		uint8_t end_index{};
		room_data_sort_kind sort_kind{};
		uint8_t flags{}; //filter conditions about room

		void on_serialize(serializer& serializer) {
			serializer += group_index;
			serializer += start_index;
			serializer += end_index;
			serializer += sort_kind;
			serializer += flags;
		}
	};

	// 238 bytes
	struct list_room_reply_message final {
		//39 bytes
		struct room_info final {
			room_id_type room_id;
			room_name_type name;
			room_flags_bit_mask::flags_type flags;
			uint8_t max_player_count;
			uint8_t current_player_count;
			datetime create_datetime;

			void on_serialize(serializer& serializer) {
				serializer += room_id;
				serializer += name;
				serializer += flags;
				serializer += max_player_count;
				serializer += current_player_count;
				serializer += create_datetime;
			}
		};

		uint8_t total_room_count{}; // the number of rooms server managing
		uint8_t result_room_count{}; // the number of rooms for request
		uint8_t reply_room_start_index{}; // the index of start room in this message
		uint8_t reply_room_end_index{}; // the index of end room in this message
		std::array<room_info, list_room_reply_room_info_count> room_info_list;

		void on_serialize(serializer& serializer) {
			serializer += total_room_count;
			serializer += result_room_count;
			serializer += reply_room_start_index;
			serializer += reply_room_end_index;
			serializer += room_info_list;
		}
	};

	// 21 bytes
	struct join_room_request_message final {
		uint8_t group_index{};
		room_id_type room_id{};
		room_password_type password{};

		void on_serialize(serializer& serializer) {
			serializer += group_index;
			serializer += room_id;
			serializer += password;
		}
	};

	//18 bytes
	struct join_room_reply_message final {
		client_address host_address{};

		void on_serialize(serializer& serializer) {
			serializer += host_address;
		}
	};

	// 6 bytes
	struct update_room_status_notice_message final {
		enum class status : uint8_t { open, close, remove };

		uint8_t group_index{};
		room_id_type room_id{};
		status status{};

		void on_serialize(serializer& serializer) {
			serializer += group_index;
			serializer += room_id;
			serializer += status;
		}
	};

	struct random_match_request_message final {
		void on_serialize(serializer& serializer [[maybe_unused]]) { }
	};
}
