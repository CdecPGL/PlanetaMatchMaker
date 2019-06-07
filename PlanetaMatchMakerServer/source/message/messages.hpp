#pragma once

#include <cstdint>

namespace pgl {
	constexpr int room_name_size = 24; // at least 8 characters with UFC-8
	constexpr int room_password_size = 16; //16 characters with ASCII

	struct room_flags_bit_mask final {
		using flags_type = uint8_t;
		const static flags_type is_private = 1;
		const static flags_type is_open = 2;
	};

	using version_type = uint16_t;
	using room_id_type = uint32_t;

	enum class message_type : uint8_t {
		authentication_request,
		authentication_reply,
		create_room_request,
		create_room_reply,
		list_room_request,
		list_room_reply,
		join_room_request,
		join_room_reply,
		update_room_status_request,
		update_room_status_reply,
		random_match_request
	};

	// 1 bytes
	struct message_header {
		message_type message_type;
	};

	// size of message should be less than 256 bytes
	struct message {};

	// 2 bytes
	struct authentication_request_message final : message {
		version_type version;
	};

	// 3 bytes
	struct authentication_reply_message final : message {
		enum class error_code : uint8_t {
			ok,
			unknown_error,
			version_mismatch,
			authentication_error,
			denied // when in black list
		};

		error_code error_code;
		version_type version;
	};

	// 42 bytes
	struct create_room_request_message final : message {
		uint8_t name[room_name_size];
		room_flags_bit_mask::flags_type flags;
		uint8_t password[room_password_size];
		uint8_t max_player_count;
	};

	// 5 bytes
	struct create_room_reply final : message {
		enum class error_code : uint8_t {
			ok,
			unknown_error,
			room_name_duplicated,
			room_count_reaches_limit
		};

		error_code error_code;
		room_id_type room_id;
	};

	// 4 bytes
	struct list_room_request_message final : message {
		enum class sort_kind : uint8_t {
			name_ascending,
			name_descending,
			date_ascending,
			date_descending
		};

		uint8_t start_index;
		uint8_t end_index;
		sort_kind sort_kind;
		uint8_t flags; //filter conditions about room
	};

	// 237 bytes
	struct list_room_reply final : message {
		enum class error_code : uint8_t {
			ok,
			unknown_error,
		};

		//39 bytes
		struct room_info {
			room_id_type room_id;
			uint8_t name[room_name_size];
			room_flags_bit_mask::flags_type flags;
			uint8_t max_player_count;
			uint8_t current_player_count;
			uint64_t date;
		};

		error_code error_code;
		uint8_t total_room_count;
		uint8_t reply_room_count;
		room_info room_info_list[6];
	};

	// 20 bytes
	struct join_room_request_message final : message {
		room_id_type room_id;
		uint8_t password[room_password_size];
	};

	// 23 bytes
	struct join_room_reply_message final : message {
		enum class error_code : uint8_t {
			ok,
			unknown_error,
			room_not_exist,
			permission_denied,
			join_rejected,
			player_count_reaches_limit
		};

		error_code error_code;
		uint32_t ip_v4_address;
		uint64_t ip_v6_address[2];
		uint16_t port_number;
	};

	// 5 bytes
	struct update_room_status_request_message final : message {
		enum class status : uint8_t { open, close, remove };

		room_id_type room_id;
		status status;
	};

	// 1 bytes
	struct update_room_status_reply_message final : message {
		enum class error_code : uint8_t {
			ok,
			unknown_error,
			room_not_exist
		};

		error_code error_code;
	};

	struct random_match_request_message final : message {
		enum class error_code : uint8_t {
			ok,
			unknown_error
		};

		error_code error_code;
	};
}
