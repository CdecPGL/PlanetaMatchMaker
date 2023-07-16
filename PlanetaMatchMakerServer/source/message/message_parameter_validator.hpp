#pragma once

#include "messages.hpp"
#include "message_handle_parameter.hpp"
#include "message_handle_utilities.hpp"

namespace pgl {
	class message_parameter_validator final {
	public:
		explicit message_parameter_validator(const std::shared_ptr<message_handle_parameter>& param);

		// Check a room id exists. If it doesn't exist, throw client error.
		void validate_room_existence(const room_data_container& room_data_container, room_id_t room_id,
			bool is_continuable = true) const;

		// Check a port number is valid. If it is not valid, throw client error.
		void validate_port_number(port_number_type port_number, bool is_continuable = true) const;

		// Check a player name is valid. If it is not valid, throw client error.
		void validate_player_name(const player_name_t& player_name, bool is_continuable = true) const;

		// Check a max player count is valid. If it is not valid, throw client error.
		void validate_max_player_count(uint8_t max_player_count, bool is_continuable = true) const;

		[[nodiscard]] const std::shared_ptr<message_handle_parameter>& get_message_handle_parameter() const;

	private:
		std::shared_ptr<message_handle_parameter> param_;
	};
}
