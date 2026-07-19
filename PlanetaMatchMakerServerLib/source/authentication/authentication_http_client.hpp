#pragma once

#include <chrono>
#include <string>

#include "authentication/authentication_execution_context.hpp"

namespace pgl::authentication_http {
	struct response final {
		unsigned status = 0;
		std::string body;
	};

	std::string append_query(std::string url, const std::string& key, const std::string& value);
	response get(const std::string& url, const authentication_execution_context& context,
		const std::string& additional_trusted_ca = {});
}
