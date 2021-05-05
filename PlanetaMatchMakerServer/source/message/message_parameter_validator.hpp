#pragma once

#include "messages.hpp"
#include "message_handle_parameter.hpp"
#include "message_handle_utilities.hpp"

namespace pgl {
	class message_parameter_validator final {
	public:
		explicit message_parameter_validator(std::shared_ptr<message_handle_parameter> param);

		// Check a room id exists. If it doesn't exist, throw server error.
		void validate_room_existence(const room_data_container& room_data_container, room_id_t room_id,
			bool is_continuable = true) const;

		// Check a port number is valid. If it is not valid, throw server error.
		void validate_port_number(port_number_type port_number, bool is_continuable = true) const;

		// Check a player name is valid. If it is not valid, throw server error.
		void validate_player_name(const player_name_t& player_name, bool is_continuable = true) const;

		// Check a max player count is valid. If it is not valid, throw server error.
		void validate_max_player_count(uint8_t max_player_count, bool is_continuable = true) const;

		const std::shared_ptr<message_handle_parameter>& get_message_handle_parameter() const;
	private:
		std::shared_ptr<message_handle_parameter> param_;
	};

	template <message_type ReplyMessageType, class TReplyMessage>
	class message_parameter_validator_with_reply final {
	public:
		explicit message_parameter_validator_with_reply(std::shared_ptr<message_handle_parameter> param) :
			message_parameter_validator_(param) {}

		// Check a room id exists. If it doesn't exist, reply error message to client and throw server error.
		void validate_room_existence(const room_data_container& room_data_container, const room_id_t room_id,
			const TReplyMessage& reply_message = {}, const bool is_continuable = true) const {
			try { message_parameter_validator_.validate_room_existence(room_data_container, room_id, is_continuable); }
			catch (server_session_error&) {
				send_reply(message_error_code::room_not_found, reply_message);
				throw;
			}
		}

		// Check a port number is valid. If it is not valid, reply error message to client and throw server error.
		void validate_port_number(const port_number_type port_number, const TReplyMessage& reply_message = {},
			const bool is_continuable = true) const {
			try { message_parameter_validator_.validate_port_number(port_number, is_continuable); }
			catch (server_session_error&) {
				send_reply(message_error_code::request_parameter_wrong, reply_message);
				throw;
			}
		}

		// Check a player name is valid. If it is not valid, reply error message to client and throw server error.
		void validate_player_name(const player_name_t& player_name, const TReplyMessage& reply_message = {},
			const bool is_continuable = true) const {
			try { message_parameter_validator_.validate_player_name(player_name, is_continuable); }
			catch (server_session_error&) {
				send_reply(message_error_code::request_parameter_wrong, reply_message);
				throw;
			}
		}

		// Check a max player count is valid. If it is not valid, reply error message to client and throw server error.
		void validate_max_player_count(const uint8_t max_player_count, const TReplyMessage& reply_message = {},
			const bool is_continuable = true) const {
			try { message_parameter_validator_.validate_max_player_count(max_player_count, is_continuable); }
			catch (server_session_error&) {
				send_reply(message_error_code::request_parameter_wrong, reply_message);
				throw;
			}
		}

	private:
		message_parameter_validator message_parameter_validator_;

		void send_reply(const message_error_code error_code, const TReplyMessage& reply_message) const {
			const reply_message_header header{
				ReplyMessageType,
				error_code
			};
			send(message_parameter_validator_.get_message_handle_parameter(), header, reply_message);
		}
	};
}
