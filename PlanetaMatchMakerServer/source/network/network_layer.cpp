#include <unordered_map>

#include "nameof.hpp"

#include "network_layer.hpp"

namespace pgl {
	ip_version string_to_ip_version(const std::string& str) {
		const static std::unordered_map<std::string, ip_version> map = {
			{std::string(nameof::nameof_enum(ip_version::v4)), ip_version::v4},
			{std::string(nameof::nameof_enum(ip_version::v6)), ip_version::v6}
		};
		return map.at(str);
	}
}
