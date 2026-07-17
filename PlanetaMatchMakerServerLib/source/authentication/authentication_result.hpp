#pragma once

#include <cstdint>

namespace pgl {
	enum class authentication_result: uint8_t {
		success,
		api_version_mismatch,
		game_id_mismatch,
		game_version_mismatch,
		unsupported_authentication_method,
		authentication_data_format_invalid,
		authentication_data_size_exceeded,
		authentication_data_invalid,
		insecure_connection,
		steam_ticket_invalid,
		steam_id_mismatch,
		steam_ownership_check_failed,
		steam_authentication_service_unavailable,
	};
}
