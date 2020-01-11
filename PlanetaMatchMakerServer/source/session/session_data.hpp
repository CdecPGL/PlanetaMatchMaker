#pragma once

#include "session/session_key.hpp"
#include "room/room_constants.hpp"
#include "network/endpoint.hpp"
#include "client/player_full_name.hpp"

namespace pgl {
	// This class need not be thread safe because one session is processed in one thread.
	// Each method may throw std::runtime_exception if errors occur.
	class session_data final {
	public:
		session_key_type generate_session_key();
		void set_hosting_room_id(room_group_index_t room_group_index, room_id_t room_id);
		void delete_hosting_room_id(room_group_index_t room_group_index, room_id_t room_id);
		void set_remote_endpoint(const endpoint& remote_endpoint);
		void set_client_player_name(player_name_t& player_name, player_tag_t player_tag);
		[[nodiscard]] room_group_index_t hosting_room_group_index() const;
		[[nodiscard]] room_id_t hosting_room_id() const;
		[[nodiscard]] bool is_session_key_generated() const;
		[[nodiscard]] bool is_hosting_room() const;
		[[nodiscard]] bool check_session_key(session_key_type session_key) const;
		// use this instead of socket.remote_endpoint() if endpoint information is required after socket is closed because socket.remote_endpoint throws exception in such situation.
		[[nodiscard]] const endpoint& remote_endpoint() const;
		[[nodiscard]] const player_full_name& client_player_name() const;
	private:
		bool is_session_key_generated_ = false;
		session_key_type session_key_{};
		bool is_hosting_room_ = false;
		room_group_index_t hosting_room_group_index_{};
		room_id_t hosting_room_id_{};
		endpoint remote_endpoint_{};
		player_full_name client_player_name_{};
	};
}
