#include <iostream>

#include "console_logger.hpp"
#include "logger_common.hpp"

using namespace std;

namespace pgl {
	console_logger::console_logger(const log_level level_threshold): logger(level_threshold) {}

	void console_logger::log(const log_level level, const string& header, const string& message) {
		ostream* os;
		switch (level) {
			case log_level::info:
			case log_level::debug:
				os = &cout;
				break;
			default:
				os = &cerr;
				break;
		}

		output_formatted_log(*os, level, header, message);
	}

	bool console_logger::is_thread_safe() const { return false; }
	bool console_logger::is_log_level_filtering_supported() const { return false; }
}
