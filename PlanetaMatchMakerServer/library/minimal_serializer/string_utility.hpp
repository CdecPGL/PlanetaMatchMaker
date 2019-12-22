/*
The MIT License (MIT)

Copyright (c) 2019 Cdec

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#pragma once

#include <sstream>
#include <string>
#include <type_traits>

#include "nameof.hpp"

namespace minimal_serializer {
	// return true if generating string from enum type is supported. If this value is false, enum type values are treated as its underlying type values. This depends on nameof C++.
	constexpr bool is_generate_enum_string_supported = nameof::is_nameof_enum_supported;

	template<typename T>
	void generate_string_converter(std::ostringstream& oss, T&& value) {
		using NonCVRefT = std::remove_cv_t<std::remove_reference_t<T>>;
		// char types is recognized as character in ostringstream, so '0' means 'end of string' and '0' in char types will not displayed.
		// To avoid this, cast char types to int before pass it to ostringstream
		if constexpr (std::is_same_v<NonCVRefT, char> || std::is_same_v<NonCVRefT, unsigned char> || std::
			is_same_v<NonCVRefT, signed char>) {
			oss << static_cast<int>(value);
		}
		else if constexpr (std::is_enum_v<NonCVRefT>) {
			if constexpr (is_generate_enum_string_supported) {
				oss << nameof::nameof_enum(value);
			}
			else {
				oss << static_cast<std::underlying_type_t<NonCVRefT>>(value);
			}
		} else {
			oss << value;
		}
	}

	template<>
	inline void generate_string_converter(std::ostringstream& oss, const std::type_info& type_info) {
		oss << type_info.name();
	}

	inline void generate_string_impl(std::ostringstream&) {}

	template <typename First, typename ... Rest>
	void generate_string_impl(std::ostringstream& oss, First&& first, Rest&& ... rest) {
		generate_string_converter(oss, std::forward<First>(first));
		generate_string_impl(oss, std::forward<Rest>(rest)...);
	}

	// Convert parameters to string and concatenate them to one string. This is thread safe.
	template <typename ... Params>
	std::string generate_string(Params&& ... params) {
		std::ostringstream oss;
		oss << std::boolalpha;
		generate_string_impl(oss, std::forward<Params>(params)...);
		return oss.str();
	}
}
