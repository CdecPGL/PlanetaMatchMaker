#pragma once
#include <string>
#include <filesystem>

#include "logger/logger.hpp"

namespace pgl {
	// use standard logger because logger using boost logger (windows, boost 1.71) crashes when client is disconnected.
	void enable_console_log(log_level level_threshold, bool use_boost_log);
	void enable_file_log(log_level level_threshold, const std::filesystem::path& file_path, bool use_boost_log);
}
