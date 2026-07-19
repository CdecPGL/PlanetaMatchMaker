#pragma once

#include "logger.hpp"

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

namespace pgl {
	// Keywords definitions for logger
	BOOST_LOG_ATTRIBUTE_KEYWORD(log_level_attribute, "Severity", log_level);
	BOOST_LOG_ATTRIBUTE_KEYWORD(log_header_attribute, "Header", std::string);

	// thread safe logger definition
	BOOST_LOG_GLOBAL_LOGGER(boost_mt_logger, boost::log::sources::severity_logger_mt<log_level>);

	boost::log::formatter get_boost_logger_default_formatter();

	// log_level to string
	std::ostream& operator<<(std::ostream& stream, const log_level level);
}
