#pragma once
#include <string>

#include "logger.hpp"

namespace pgl {
	void output_formatted_log(std::ostream& out_stream, log_level level, const std::string& header, const std::string& message);
}
