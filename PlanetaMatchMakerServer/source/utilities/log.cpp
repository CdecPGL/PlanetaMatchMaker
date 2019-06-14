#include <iostream>
#include <shared_mutex>

#include "nameof.hpp"

#include "datetime/datetime.hpp"
#include "log.hpp"

using namespace std;
using namespace boost;

namespace pgl {
	static log_level output_log_level = log_level::info;
	static shared_mutex output_log_level_mutex;
	static mutex output_mutex;

	void log_impl_without_mutex(const log_level level, string&& log_header, string&& log_body) {
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

		*os << "[" << get_now_time_string() << "] " << level << log_header << ": " << log_body << endl;
	}

	void set_output_log_level(const log_level level) {
		std::scoped_lock locks{output_log_level_mutex, output_mutex};
		output_log_level = level;
		log_impl_without_mutex(log_level::info, "", generate_string("Output log level is set to \"", level, "\"."));
	}

	void log_impl(const log_level level, string&& log_header, string&& log_body) {
		if (level < output_log_level) {
			return;
		}

		shared_lock output_log_level_lock(output_log_level_mutex);
		lock_guard output_lock(output_mutex);

		log_impl_without_mutex(level, std::move(log_header), std::move(log_body));
	}
}
