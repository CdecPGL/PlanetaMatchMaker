#include <iostream>

#include "server/server.hpp"
#include "utilities/file_utilities.hpp"

using namespace std;
using namespace pgl;
using namespace boost;

int main(const int argc, char* argv[]) {
	try {
		const auto setting_file_path = argc > 1
											? std::filesystem::path(argv[1])
											: get_application_setting_directory() / "setting.json";
		server server(setting_file_path);
		server.run();
	} catch (const std::exception& e) {
		cerr << "Fatal error occured: " << e.what() << endl;
		exit(1);
	}
	catch (...) {
		cerr << "Fatal error occured." << endl;
		exit(1);
	}
}
