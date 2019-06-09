#pragma once

#include "string_utility.hpp"
#include "asio_stream_compatibility.hpp"

namespace pgl {
	enum class log_level { debug, info, warning, error, fatal };

	// Set threshold of log level thread safely to output.
	void set_output_log_level(log_level level);

	// Implementation of thread safe log functions.
	void log_impl(log_level level, std::string&& log_header, std::string&& log_body);

	// Log thread safely.
	template <typename ... Params>
	void log(const log_level level, Params&& ... params) {
		log_impl(level, "", generate_string(params...));
	}

	// Log thread safely with information of endpoint.
	template <typename ... Params>
	void log_with_endpoint(const log_level level,
	                       const boost::asio::basic_socket<boost::asio::ip::tcp>::endpoint_type& endpoint,
	                       Params&& ... params) {
		log_impl(level, generate_string(" @", endpoint), generate_string(params...));
	}
}
