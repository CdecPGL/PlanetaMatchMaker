#pragma once

#include "room/room_constants.hpp"
#include "network/endpoint.hpp"
#include "client/player_full_name.hpp"

namespace pgl {
	// This class need not be thread safe because one session is processed in one thread.
	// Each method may throw std::runtime_exception if errors occur.
	class session_data final {
	public:
		void set_hosting_room_id(room_id_t room_id);
		void delete_hosting_room_id(room_id_t room_id);
		void set_remote_endpoint(const endpoint& remote_endpoint);
		void set_client_player_name(const player_full_name& player_full_name);
		void set_authenticated();

		[[nodiscard]] room_id_t hosting_room_id() const;
		[[nodiscard]] bool is_hosting_room() const;
		// use this instead of socket.remote_endpoint() if endpoint information is required after socket is closed because socket.remote_endpoint throws exception in such situation.
		[[nodiscard]] const endpoint& remote_endpoint() const;
		[[nodiscard]] const player_full_name& client_player_name() const;
		[[nodiscard]] bool is_authenticated() const;

	private:
		bool is_authenticated_ = false;
		bool is_hosting_room_ = false;
		room_id_t hosting_room_id_{};
		endpoint remote_endpoint_{};
		player_full_name client_player_name_{};
	};
}
