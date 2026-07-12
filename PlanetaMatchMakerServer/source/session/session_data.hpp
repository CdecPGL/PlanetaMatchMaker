#pragma once

#include <optional>
#include <string>

#include "authentication/authentication_method.hpp"
#include "room/room_constants.hpp"
#include "network/endpoint.hpp"
#include "client/player_full_name.hpp"
#include "session_constants.hpp"

namespace pgl {
	struct authenticated_identity final {
		authentication_method method = authentication_method::oidc;
		std::string verified_user_id;
		std::optional<game_host_external_id_t> external_id;
		std::string display_name;
		std::string oidc_issuer;
		std::string oidc_subject;
		std::string oidc_audience;
	};

	// This class need not be thread safe because one session is processed serially through its session strand.
	// Do not access it from outside the owning session strand.
	// Each method may throw std::runtime_exception if errors occur.
	class session_data final {
	public:
		void set_session_number(session_number_t session_number);
		void set_hosting_room_id(room_id_t room_id);
		void delete_hosting_room_id(room_id_t room_id);
		void set_remote_endpoint(const endpoint& remote_endpoint);
		void set_client_player_name(const player_full_name& player_full_name);
		void set_authenticated();
		void set_authenticated(const authenticated_identity& identity);

		[[nodiscard]] std::optional<session_number_t> session_number() const;
		[[nodiscard]] room_id_t hosting_room_id() const;
		[[nodiscard]] bool is_hosting_room() const;
		// use this instead of socket.remote_endpoint() if endpoint information is required after socket is closed because socket.remote_endpoint throws exception in such situation.
		[[nodiscard]] const endpoint& remote_endpoint() const;
		[[nodiscard]] const player_full_name& client_player_name() const;
		[[nodiscard]] bool is_authenticated() const;
		[[nodiscard]] const std::optional<authenticated_identity>& identity() const;

	private:
		bool is_authenticated_ = false;
		bool is_hosting_room_ = false;
		std::optional<session_number_t> session_number_{};
		room_id_t hosting_room_id_{};
		endpoint remote_endpoint_{};
		player_full_name client_player_name_{};
		std::optional<authenticated_identity> identity_{};
	};
}
