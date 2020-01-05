#pragma once

#include "logger.hpp"

namespace pgl {
	class boost_console_logger final : public logger {
	public:
		explicit boost_console_logger(log_level level_threshold);
		virtual ~boost_console_logger() = default;
		void log(log_level level, const std::string& header, const std::string& message) override;
		[[nodiscard]] bool is_thread_safe() const override;
		[[nodiscard]] bool is_log_level_filtering_supported() const override;
	};
}
