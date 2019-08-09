#include <cstdlib>
#include <cstring>
#include <filesystem>

#include <boost/predef.h>

# include "file_utilities.hpp"

namespace pgl {
	constexpr size_t max_path_length = 256;
	constexpr char application_name[] = ".pmms";

	std::filesystem::path get_home_directory() {
#if BOOST_OS_WINDOWS
		size_t length;
		char buffer_drive[max_path_length];
		char buffer_path[max_path_length];
		getenv_s(&length, buffer_drive, "HOMEDRIVE");
		getenv_s(&length, buffer_path, "HOMEPATH");
		return std::string(buffer_drive) + buffer_path;
#else
		auto ref = std::getenv("HOME");
		if(std::strlen(ref) + 1 > max_path_length) {
			throw std::out_of_range("A length of home path is too long.");
		}
		char buffer[max_path_length];
		std::strcpy(buffer, ref);
		return buffer;
#endif
	}

	std::filesystem::path get_application_setting_directory() {
		const auto home_path(get_home_directory());
		return home_path / application_name;
	}

	std::filesystem::path get_or_create_application_setting_directory() {
		const auto application_path(get_application_setting_directory());
		if (!exists(application_path)) {
			create_directory(application_path);
		}
		return application_path;
	}
}
