#include <cstdlib>
#include <cstring>
#include <filesystem>

#include <boost/predef.h>

#include "file_utilities.hpp"
#include "application.hpp"

using namespace std;

namespace pgl {
	constexpr size_t max_path_length = 256;

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
#if BOOST_OS_WINDOWS
		const auto system_setting_directory = filesystem::path("C:\\");
#else
		const auto system_setting_directory = filesystem::path("/etc");
#endif
		return system_setting_directory / application::application_short_name;
	}

	std::filesystem::path get_application_log_directory() {
#if BOOST_OS_WINDOWS
		const auto system_log_directory = filesystem::path("C:\\log");
#else
		const auto system_log_directory = filesystem::path("/var/log");
#endif
		return system_log_directory;
	}

	std::filesystem::path get_or_create_application_setting_directory() {
		const auto setting_path(get_application_setting_directory());
		if (!exists(setting_path)) { create_directory(setting_path); }
		return setting_path;
	}

	std::filesystem::path get_or_create_application_log_directory() {
		const auto log_directory(get_application_log_directory());
		if (!exists(log_directory)) { create_directory(log_directory); }
		return log_directory;
	}
}
