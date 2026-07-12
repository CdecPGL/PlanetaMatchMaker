#pragma once
#include <string>

namespace pgl {
	enum class log_level { debug, info, warning, error, fatal };

	class logger {
	public:
		virtual ~logger() = default;
		virtual void log(log_level level, const std::string& header, const std::string& message) = 0;
		[[nodiscard]] virtual bool is_thread_safe() const = 0;
		[[nodiscard]] virtual bool is_log_level_filtering_supported() const = 0;
		[[nodiscard]] log_level level_threshold() const { return level_threshold_; }

	protected:
		explicit logger(const log_level level_threshold) : level_threshold_(level_threshold) {}

	private:
		const log_level level_threshold_;
	};
}