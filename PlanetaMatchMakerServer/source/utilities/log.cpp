#include <iostream>

#include "nameof.hpp"

#include "datetime/datetime.hpp"
#include "log.hpp"

using namespace std;
using namespace boost;

namespace pgl {
	static log_level output_log_level = log_level::info;

	void set_output_log_level(const log_level level) {
		output_log_level = level;
		log_impl(log_level::info, true, "",
		         generate_string("Output log level is set to \"", NAMEOF_ENUM(level), "\"."));
	}

	void log_impl(const log_level level, const bool force, string&& log_header, string&& log_body) {
		if (!force && level < output_log_level) {
			return;
		}

		ostream* os = nullptr;
		switch (level) {
		case log_level::info:
		case log_level::debug:
			os = &cout;
			break;
		default: 
			os = &cerr;
			break;
		}

		*os << "[" << get_time_string() << "] " << NAMEOF_ENUM(level) << log_header << ": " << log_body << endl;
	}
}
