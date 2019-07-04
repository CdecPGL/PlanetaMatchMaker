#pragma once

#include <cstdint>

namespace pgl {
	// This class need not be thread safe because used for only read access.
	struct server_setting final {
		bool enable_session_key_check;
		uint32_t time_out_seconds;
		uint32_t max_connections_per_thread;
		uint32_t thread_count;
	};
}