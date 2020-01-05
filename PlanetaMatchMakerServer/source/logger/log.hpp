#pragma once

#include "minimal_serializer/string_utility.hpp"

#include "logger.hpp"
#include "utilities/asio_stream_compatibility.hpp"

namespace pgl {
	// convert string to log level thead safely. If str is invalid, throws std::out_of_range.
	log_level string_to_log_level(const std::string& str);

	// Add logger. (not thread safe)
	void add_logger(std::unique_ptr<logger>&& logger);

	// Implementation of thread safe log function.
	void log_impl(log_level level, std::string&& header, std::string&& message);

	// Log thread safely.
	template <typename ... Params>
	void log(const log_level level, Params&& ... params) {
		log_impl(level, "", minimal_serializer::generate_string(params...));
	}

	// Log thread safely with information of endpoint.
	template <typename ... Params>
	void log_with_endpoint(const log_level level,
		const boost::asio::basic_socket<boost::asio::ip::tcp>::endpoint_type& endpoint,
		Params&& ... params) {
		log_impl(level, minimal_serializer::generate_string(" @", endpoint),
			minimal_serializer::generate_string(params...));
	}
}
