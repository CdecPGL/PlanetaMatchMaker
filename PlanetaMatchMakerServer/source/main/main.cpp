#include <iostream>

#include "log_initializer.hpp"

#include "server/server.hpp"
#include "utilities/file_utilities.hpp"
#include "utilities/application.hpp"

using namespace std;
using namespace pgl;
using namespace boost;

int main(const int argc, char* argv[]) {
	try {
		const auto setting_file_path = argc > 1
			                               ? std::filesystem::path(argv[1])
			                               : get_application_setting_directory() / application::setting_file_name;

		// Load setting
		auto setting = std::make_unique<server_setting>();
		setting->load_from_setting_file(setting_file_path);

		// Setup log
		if (setting->log.enable_console_log) {
			enable_console_log(setting->log.console_log_level, false);
		}
		if (setting->log.enable_file_log) {
			enable_file_log(setting->log.file_log_level, setting->log.file_log_path, false);
		}

		// Output setting
		log(log_level::info, "Server setting is loaded.");
		setting->output_to_log();

		// Start server
		server server(std::move(setting));
		server.run();
	}
	catch (const std::exception& e) {
		cerr << "Fatal error occured: " << e.what() << endl;
		exit(1);
	}
	catch (...) {
		cerr << "Fatal error occured." << endl;
		exit(1);
	}
}
