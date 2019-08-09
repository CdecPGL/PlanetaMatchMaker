#pragma once

#include <string>
#include <filesystem>

namespace pgl {
	std::filesystem::path get_home_directory();
	std::filesystem::path get_application_setting_directory();
	std::filesystem::path get_or_create_application_setting_directory();
}