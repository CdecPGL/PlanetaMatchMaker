#include <iostream>
#include <shared_mutex>
#include <unordered_map>

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

		*os << "[" << get_now_datetime_string() << "] " << level << log_header << ": " << log_body << endl;
	}

	log_level string_to_log_level(const std::string& str) {
		const static std::unordered_map<std::string, log_level> map{
			{std::string(nameof::nameof_enum(log_level::debug)), log_level::debug},
			{std::string(nameof::nameof_enum(log_level::info)), log_level::info},
			{std::string(nameof::nameof_enum(log_level::warning)), log_level::warning},
			{std::string(nameof::nameof_enum(log_level::error)), log_level::error},
			{std::string(nameof::nameof_enum(log_level::fatal)), log_level::fatal},
		};
		return map.at(str);
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
