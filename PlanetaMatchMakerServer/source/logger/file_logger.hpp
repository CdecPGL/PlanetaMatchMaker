#pragma once

#include <fstream>
#include <filesystem>

#include "logger.hpp"

namespace pgl {
	class file_logger final : public logger {
	public:
		explicit file_logger(log_level level_threshold, const std::filesystem::path& file_path);
		~file_logger() override = default;
		void log(log_level level, const std::string& header, const std::string& message) override;
		[[nodiscard]] bool is_thread_safe() const override;
		[[nodiscard]] bool is_log_level_filtering_supported() const override;
	private:
		std::ofstream out_stream_;
	};
}
