#include <filesystem>
#include <iostream>

#include "log_initializer.hpp"

#include "logger/log.hpp"
#include "logger/console_logger.hpp"
#include "logger/file_logger.hpp"
#include "logger/boost_console_logger.hpp"
#include "logger/boost_file_logger.hpp"
#include "utilities/file_utilities.hpp"
#include "utilities/application.hpp"

using namespace std;

namespace pgl {
	void enable_console_log(log_level level_threshold, const bool use_boost_log) {
		if (use_boost_log) { add_logger(std::make_unique<boost_console_logger>(level_threshold)); }
		else { add_logger(std::make_unique<console_logger>(level_threshold)); }
	}

	void enable_file_log(log_level level_threshold, const std::string& file_path, const bool use_boost_log) {
		filesystem::path actual_file_path(file_path);

		// use default path if indicated path is empty.
		if (actual_file_path.empty()) {
			actual_file_path = get_or_create_application_log_directory() / application::log_file_name;
		}

		try {
			if (!exists(actual_file_path.parent_path())) {
				cerr << "The directory to output log file \"" << actual_file_path << "\" does not exist." << endl;
				return;
			}

			if (use_boost_log) {
				add_logger(std::make_unique<boost_file_logger>(level_threshold, actual_file_path.string()));
			}
			else { add_logger(std::make_unique<file_logger>(level_threshold, actual_file_path.string())); }
		}
		catch (exception& e) { cerr << "Failed to open a log file " << actual_file_path << ": " << e.what() << endl; }
	}
}
