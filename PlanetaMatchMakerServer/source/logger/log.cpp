#include <iostream>
#include <shared_mutex>
#include <unordered_map>

#include <boost/log/expressions.hpp>
#include <boost/log/keywords/auto_flush.hpp>
#include <boost/log/common.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/severity_feature.hpp>
#include <boost/log/sources/global_logger_storage.hpp>
#include <boost/log/core/record.hpp>
#include <boost/log/sinks.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/expressions/formatters.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/manipulators/add_value.hpp>

#include "nameof.hpp"

#include "log.hpp"

using namespace std;
using namespace boost;
using namespace minimal_serializer;

namespace pgl {
	// Keywords definitions for logger
	BOOST_LOG_ATTRIBUTE_KEYWORD(log_level_attribute, "Severity", log_level);
	BOOST_LOG_ATTRIBUTE_KEYWORD(log_header_attribute, "Header", std::string);

	// thread safe logger definition
	BOOST_LOG_GLOBAL_LOGGER(standard_logger, boost::log::sources::severity_logger_mt<log_level>);

	BOOST_LOG_GLOBAL_LOGGER_INIT(standard_logger, boost::log::sources::severity_logger_mt<log_level>) {
		auto logger = boost::log::sources::severity_logger_mt<log_level>();
		logger.add_attribute("TimeStamp", log::attributes::local_clock());
		logger.add_attribute("ThreadID", log::attributes::current_thread_id());
		return logger;
	}

	// log_level to string
	std::ostream& operator<<(std::ostream& stream, const log_level level) {
		stream << nameof::nameof_enum(level);
		return stream;
	}

	void initialize_log(log_level level, const bool enable_console_log, const bool enable_file_log,
		const std::string& file_log_path) {
		auto format = log::expressions::format("[%1%] [%2%] %3%%4%: %5%")
			% log::expressions::format_date_time<posix_time::ptime>(
				"TimeStamp", "%Y-%m-%d %H:%M:%S.%f")
			% log::expressions::attr<log::attributes::current_thread_id::value_type>("ThreadID")
			% log_level_attribute
			% log_header_attribute
			% log::expressions::message;

		if (enable_console_log) {
			log::add_console_log(std::cout,
				log::keywords::auto_flush = true,
				log::keywords::filter = log_level_attribute >= level,
				log::keywords::format = format
			);
		}

		if (enable_file_log) {
			log::add_file_log(
				log::keywords::auto_flush = true,
				log::keywords::filter = log_level_attribute >= level,
				log::keywords::open_mode = std::ios::app,
				log::keywords::file_name = file_log_path,
				log::keywords::format = format
			);
		}

		log_impl(log_level::info, "", generate_string("Output log level is set to \"", level, "\"."));
	}

	void log_impl(const log_level level, string&& log_header, string&& log_body) {
		BOOST_LOG_SEV(standard_logger::get(), level) << log::add_value("Header", log_header) << log_body;
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
		initialize_log(level, true, false, "/var/log/pmms.log");
	}
}
