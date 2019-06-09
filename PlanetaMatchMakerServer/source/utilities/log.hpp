#pragma once

#include "string_utility.hpp"
#include "asio_stream_compatibility.hpp"

namespace pgl {
	enum class log_level { debug, info, warning, error, fatal };

	std::string get_time_string();
	void set_output_log_level(log_level level);

	void log_impl(log_level level, bool force, std::string&& log_header, std::string&& log_body);

	template <typename ... Params>
	void log(const log_level level, Params&& ... params) {
		log_impl(level, false, "", generate_string(params...));
	}

	template <typename ... Params>
	void log_with_endpoint(const log_level level,
	                       const boost::asio::basic_socket<boost::asio::ip::tcp>::endpoint_type& endpoint,
	                       Params&& ... params) {
		log_impl(level, false, generate_string(" @", endpoint), generate_string(params...));
	}
}
