#include "boost_file_logger.hpp"
#include "boost_logger_common.hpp"

#include "nameof.hpp"

using namespace boost;

namespace pgl {
	boost_file_logger::
	boost_file_logger(const log_level level_threshold, const std::string& file_path) : logger(level_threshold) {
		add_file_log(
			log::keywords::auto_flush = true,
			log::keywords::filter = log_level_attribute >= this->level_threshold(),
			log::keywords::open_mode = std::ios::app,
			log::keywords::file_name = file_path,
			log::keywords::format = get_boost_logger_default_formatter()
		);
	}

	void boost_file_logger::log(log_level level, const std::string& header, const std::string& message) {
		BOOST_LOG_SEV(boost_mt_logger::get(), level) << log::add_value("Header", header) << message;
	}

	bool boost_file_logger::is_thread_safe() const { return true; }
	bool boost_file_logger::is_log_level_filtering_supported() const { return true; }
}
