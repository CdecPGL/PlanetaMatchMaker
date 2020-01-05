#include "boost_logger_common.hpp"

using namespace boost;

namespace pgl {
	BOOST_LOG_GLOBAL_LOGGER_INIT(boost_mt_logger, boost::log::sources::severity_logger_mt<log_level>) {
		auto logger = boost::log::sources::severity_logger_mt<log_level>();
		logger.add_attribute("TimeStamp", log::attributes::local_clock());
		logger.add_attribute("ThreadID", log::attributes::current_thread_id());
		return logger;
	}

	log::formatter get_boost_logger_default_formatter() {
		return log::expressions::format("[%1%] [%2%] %3%%4%: %5%")
			% log::expressions::format_date_time<posix_time::ptime>(
				"TimeStamp", "%Y-%m-%d %H:%M:%S.%f")
			% log::expressions::attr<log::attributes::current_thread_id::value_type>("ThreadID")
			% log_level_attribute
			% log_header_attribute
			% log::expressions::message;
	}

	std::ostream& operator<<(std::ostream& stream, const log_level level) {
		stream << nameof::nameof_enum(level);
		return stream;
	}
}
