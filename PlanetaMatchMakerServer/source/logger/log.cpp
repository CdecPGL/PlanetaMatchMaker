#include <unordered_map>
#include <algorithm>

#include "log.hpp"

using namespace std;
using namespace boost;
using namespace minimal_serializer;

namespace pgl {
	static mutex output_mutex;
	std::vector<std::unique_ptr<logger>> loggers;
	bool are_all_loggers_thread_safe = true;

	void add_logger(std::unique_ptr<logger>&& logger) {
		are_all_loggers_thread_safe &= logger->is_thread_safe();
		loggers.push_back(std::move(logger));
	}

	void log_impl(const log_level level, string&& header, string&& message) {
		std::vector<logger*> active_loggers;
		active_loggers.reserve(loggers.size());
		for (auto&& logger : loggers) {
			// We filter logs by level in logger if logger supports log level filtering
			// If not, judge there
			if (logger->is_log_level_filtering_supported() || level >= logger->level_threshold()) {
				active_loggers.push_back(logger.get());
			}
		}

		// do nothing if there are no active loggers to avoid cost of mutex lock
		if (active_loggers.empty()) { return; }

		if (are_all_loggers_thread_safe) {
			for (auto&& logger : active_loggers) { logger->log(level, header, message); }
		}
		else {
			lock_guard output_lock(output_mutex);
			for (auto&& logger : active_loggers) { logger->log(level, header, message); }
		}
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
}
