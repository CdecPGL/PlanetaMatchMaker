#pragma once

#include "client_constants.hpp"

namespace pgl {
	// 26 bytes
	struct player_full_name final {
		player_name_t name;
		player_tag_t tag;

		[[nodiscard]] std::string generate_full_name() const;

		void on_serialize(minimal_serializer::serializer& serializer) {
			serializer += name;
			serializer += tag;
		}
	};
}
