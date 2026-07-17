#include "authentication_method.hpp"

#include <ostream>
#include <stdexcept>
#include <unordered_map>

#include "nameof.hpp"

namespace pgl {
	authentication_method string_to_authentication_method(const std::string& str) {
		const static std::unordered_map<std::string, authentication_method> map{
			{std::string(nameof::nameof_enum(authentication_method::steam)), authentication_method::steam},
			{std::string(nameof::nameof_enum(authentication_method::oidc)), authentication_method::oidc},
			{std::string(nameof::nameof_enum(authentication_method::none)), authentication_method::none},
		};
		return map.at(str);
	}

	std::string authentication_method_to_string(const authentication_method method) {
		return std::string(nameof::nameof_enum(method));
	}

	std::ostream& operator<<(std::ostream& os, const authentication_method method) {
		os << authentication_method_to_string(method);
		return os;
	}
}
