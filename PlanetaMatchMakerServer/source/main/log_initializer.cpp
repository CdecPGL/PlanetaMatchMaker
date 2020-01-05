#include <filesystem>
#include <iostream>

#include "log_initializer.hpp"

#include "logger/log.hpp"
#include "logger/console_logger.hpp"
#include "logger/file_logger.hpp"
#include "logger/boost_console_logger.hpp"
#include "logger/boost_file_logger.hpp"

using namespace std;

namespace pgl {
	void enable_console_log(log_level level_threshold, const bool use_boost_log) {
		if (use_boost_log) { add_logger(std::make_unique<boost_console_logger>(level_threshold)); }
		else { add_logger(std::make_unique<console_logger>(level_threshold)); }
	}

	void enable_file_log(log_level level_threshold, std::string& file_path, const bool use_boost_log) {
		const auto path = filesystem::path(file_path);
		try {
			if (!exists(path.parent_path())) {
				cerr << "The directory to output log file \"" << path << "\" does not exist." << endl;
				return;
			}

			if (use_boost_log) { add_logger(std::make_unique<boost_file_logger>(level_threshold, file_path)); }
			else { add_logger(std::make_unique<file_logger>(level_threshold, file_path)); }
		}
		catch (exception& e) { cerr << "Failed to initialize log system for \"" << path << "\": " << e.what() << endl; }
	}
}
