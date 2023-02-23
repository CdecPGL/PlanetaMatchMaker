#pragma once

#include <string>
#include <boost/lexical_cast.hpp>

#include "minimal_serializer/string_utility.hpp"

#include "utilities/checked_static_cast.hpp"

namespace pgl {
	/**
	 * Get environment variable.
	 * If there is no environment variable with passed name, this function do nothing.
	 * 
	 * @param var_name A name of environment variable to get
	 * @param dest A destination to write gotten gotten environment variable
	 * @return true if the environment variable exists and destination is overwritten
 	 * @tparam T A type of destination variable
 	 * @exception std::runtime_error Failed to get existing environment variable or failed to convert environment variable to destination type
 	 * @note This function is not thread safe
	*/
	template <typename T>
	bool get_env_var(const std::string& var_name, T& dest) {
		// For compatibility of clang and gcc, we don't use secure version by MSVC like getenv_s
		// The return value of getenv must be only used for temporal reference
		// ReSharper disable once CppDeprecatedEntity
		const auto* temp_value = getenv(var_name.c_str());  // NOLINT(concurrency-mt-unsafe)

		if(temp_value == nullptr) {
			return false;
		}

		// copy and construct string from environment variable value
		const std::string value_str(temp_value);
		try { dest = boost::lexical_cast<T>(value_str); }
		catch (const boost::bad_lexical_cast& e) {
			throw std::runtime_error(minimal_serializer::generate_string("The environment variable \"", var_name, "=",
				value_str, "\" is not convertible to ", nameof::nameof_type<T>(), " (", e.what(), ")"));
		}
		return true;
	}

	template <>
	inline bool get_env_var<bool>(const std::string& var_name, bool& dest) {
		std::string str;
		if (!get_env_var(var_name, str)) { return false; }
		auto lower_str = str;
		std::ranges::transform(lower_str, lower_str.begin(), [](const char c) {
			return static_cast<char>(tolower(c));
		});
		if (lower_str == "true") { dest = true; }
		else if (lower_str == "false") { dest = false; }
		else {
			throw std::runtime_error(minimal_serializer::generate_string("The environment variable \"", var_name, "=",
				str, "\" is not convertible to bool (true or false)."));
		}

		return true;
	}

	template <>
	inline bool get_env_var<uint8_t>(const std::string& var_name, uint8_t& dest) {
		int32_t v;
		if (!get_env_var(var_name, v)) { return false; }
		try { dest = range_checked_static_cast<uint8_t>(v); }
		catch (const static_cast_range_error& e) {
			throw std::runtime_error(minimal_serializer::generate_string("The environment variable \"", var_name, "=",
				v, "\" is not convertible to uint8_t.", e.what()));
		}

		return true;
	}

	template <>
	inline bool get_env_var<int8_t>(const std::string& var_name, int8_t& dest) {
		int32_t v;
		if (!get_env_var(var_name, v)) { return false; }
		try { dest = range_checked_static_cast<int8_t>(v); }
		catch (const static_cast_range_error& e) {
			throw std::runtime_error(minimal_serializer::generate_string("The environment variable \"", var_name, "=",
				v, "\" is not convertible to uint8_t.", e.what()));
		}

		return true;
	}
}