#pragma once

#include "minimal_serializer/string_utility.hpp"
#include "utilities/asio_stream_compatibility.hpp"

namespace pgl {
	enum class log_level { debug, info, warning, error, fatal };

	// convert string to log level thead safely. If str is invalid, throws std::out_of_range.
	log_level string_to_log_level(const std::string& str);

	// Set threshold of log level thread safely to output.
	void set_output_log_level(log_level level);

	// Initialize log system (not thread safe)
	void initialize_log(log_level level, bool enable_console_log, bool enable_file_log, const std::string& file_log_path);

	// Implementation of thread safe log functions.
	void log_impl(log_level level, std::string&& log_header, std::string&& log_body);

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
		log_impl(level, minimal_serializer::generate_string(" @", endpoint), minimal_serializer::generate_string(params...));
	}
}
