#include "session/session_data.hpp"
#include "session/session_key.hpp"

namespace pgl {
	session_key_type session_data::generate_session_key() {
		if (is_session_key_generated()) {
			throw std::runtime_error("A session key can't be generated multi times.");
		}

		session_key_ = generate_random_session_key();
		is_session_key_generated_ = true;
		return session_key_;
	}

	void session_data::set_hosting_room_id(const room_group_index_type room_group_index, const room_id_type room_id) {
		if (is_hosting_room_) {
			throw std::runtime_error("A hosting room is already set.");
		}

		hosting_room_group_index_ = room_group_index;
		hosting_room_id_ = room_id;
		is_hosting_room_ = true;
	}

	void session_data::delete_hosting_room_id(const room_group_index_type room_group_index,
		const room_id_type room_id) {
		if (!is_hosting_room_) {
			throw std::runtime_error("A hosting room is not set.");
		}
		if (hosting_room_group_index_ != room_group_index) {
			throw std::runtime_error(minimal_serializer::generate_string("A passed room group index(", room_group_index,
				") is different from hosting room group index(", hosting_room_group_index_, ")."));
		}
		if (hosting_room_id_ != room_id) {
			throw std::runtime_error(minimal_serializer::generate_string("A passed room id(", room_id,
				") is different from hosting room id(", hosting_room_id_, ")."));
		}

		is_hosting_room_ = false;
	}

	room_group_index_type session_data::hosting_room_group_index() const {
		if (!is_hosting_room_) {
			throw std::runtime_error("A hosting room is not set.");
		}

		return hosting_room_group_index_;
	}

	room_id_type session_data::hosting_room_id() const {
		if (!is_hosting_room_) {
			throw std::runtime_error("A hosting room is not set.");
		}

		return hosting_room_id_;
	}

	bool session_data::is_session_key_generated() const {
		return is_session_key_generated_;
	}

	bool session_data::is_hosting_room() const {
		return is_hosting_room_;
	}

	bool session_data::check_session_key(const session_key_type session_key) const {
		if (!is_session_key_generated_) {
			throw std::runtime_error("A session key is not generated.");
		}

		return is_session_key_generated_ && session_key_ == session_key;
	}
}
