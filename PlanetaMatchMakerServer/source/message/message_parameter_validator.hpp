#pragma once

#include "messages.hpp"
#include "message_handle_parameter.hpp"
#include "message_handle_utilities.hpp"

namespace pgl {
	class message_parameter_validator final {
	public:
		explicit message_parameter_validator(std::shared_ptr<message_handle_parameter> param);

		// Check a room group index is valid. If it is not valid, throw server error.
		void validate_room_group_existence(room_group_index_t room_group_index) const;

		// Check a room id exists. If it doesn't exist, throw server error.
		void validate_room_existence(const room_data_container& room_data_container, room_id_t room_id) const;

		// Check a port number is valid. If it is not valid, throw server error.
		void validate_port_number(port_number_type port_number) const;

		const std::shared_ptr<message_handle_parameter>& get_message_handle_parameter() const;
	private:
		std::shared_ptr<message_handle_parameter> param_;
	};

	template <message_type ReplyMessageType, class TReplyMessage>
	class message_parameter_validator_with_reply final {
	public:
		explicit message_parameter_validator_with_reply(std::shared_ptr<message_handle_parameter> param) :
			message_parameter_validator_(param) {}

		// Check a room group index is valid. If it is not valid, reply error message to client and throw server error.
		void validate_room_group_existence(const room_group_index_t room_group_index,
			const TReplyMessage& reply_message = {}) const {
			try { message_parameter_validator_.validate_room_group_existence(room_group_index); }
			catch (server_session_error&) {
				const reply_message_header header{
					ReplyMessageType,
					message_error_code::room_group_not_found
				};
				send(message_parameter_validator_.get_message_handle_parameter(), header, reply_message);
				const auto error_message = minimal_serializer::generate_string("The room group with index \"",
					room_group_index, "\" does not exist.");
				throw;
			}
		}

		// Check a room id exists. If it doesn't exist, reply error message to client and throw server error.
		void validate_room_existence(const room_data_container& room_data_container, const room_id_t room_id,
			const TReplyMessage& reply_message = {}) const {
			try { message_parameter_validator_.validate_room_existence(room_data_container, room_id); }
			catch (server_session_error&) {
				// Send room doesn't exist error to the client
				const reply_message_header header{
					ReplyMessageType,
					message_error_code::room_not_found
				};
				send(message_parameter_validator_.get_message_handle_parameter(), header, reply_message);
				const auto error_message = minimal_serializer::generate_string("The room with id \"", room_id,
					"\" does not exist.");
				throw;
			}
		}

		// Check a port number is valid. If it is not valid, reply error message to client and throw server error.
		void validate_port_number(port_number_type port_number, const TReplyMessage& reply_message = {}) const {
			try { message_parameter_validator_.validate_port_number(port_number); }
			catch (server_session_error&) {
				const reply_message_header header{
					ReplyMessageType,
					message_error_code::request_parameter_wrong
				};
				send(message_parameter_validator_.get_message_handle_parameter(), header, reply_message);
				const auto error_message = minimal_serializer::generate_string("The port number \"", port_number,
					"\" is invalid.");
				throw;
			}
		}

	private:
		message_parameter_validator message_parameter_validator_;
	};
}
