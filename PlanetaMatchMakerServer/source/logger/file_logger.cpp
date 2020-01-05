#include <fstream>

#include "file_logger.hpp"
#include "logger_common.hpp"

using namespace std;

namespace pgl {
	file_logger::file_logger(const log_level level_threshold, const string& file_path) : logger(level_threshold),
		out_stream_(file_path, std::ios_base::out | std::ios::app) {
		if (!out_stream_) { throw runtime_error("Failed to open a log file."); }
	}

	void file_logger::log(const log_level level, const string& header, const string& message) {
		output_formatted_log(out_stream_, level, header, message);
	}

	bool file_logger::is_thread_safe() const { return false; }
	bool file_logger::is_log_level_filtering_supported() const { return false; }
}
