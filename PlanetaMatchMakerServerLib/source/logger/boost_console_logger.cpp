#include "boost_console_logger.hpp"
#include "boost_logger_common.hpp"

#include "nameof.hpp"

using namespace boost;

namespace pgl {
	boost_console_logger::boost_console_logger(const log_level level_threshold): logger(level_threshold) {
		add_console_log(std::cout,
			log::keywords::auto_flush = true,
			log::keywords::filter = log_level_attribute >= this->level_threshold(),
			log::keywords::format = get_boost_logger_default_formatter()
		);
	}

	void boost_console_logger::log(log_level level, const std::string& header, const std::string& message) {
		BOOST_LOG_SEV(boost_mt_logger::get(), level) << log::add_value("Header", header) << message;
	}

	bool boost_console_logger::is_thread_safe() const { return true; }
	bool boost_console_logger::is_log_level_filtering_supported() const { return true; }
}
