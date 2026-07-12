#pragma once

#include <chrono>
#include <string>

namespace pgl::authentication_http {
	struct response final {
		unsigned status = 0;
		std::string body;
	};

	std::string append_query(std::string url, const std::string& key, const std::string& value);
	response get(const std::string& url, std::chrono::seconds timeout);
}
