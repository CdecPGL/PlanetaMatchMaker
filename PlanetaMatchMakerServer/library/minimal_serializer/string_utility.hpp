/*
The MIT License (MIT)

Copyright (c) 2019-2022 Cdec

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#pragma once

#include <sstream>
#include <string>
#include <type_traits>
#include <vector>
#include <array>

#include "nameof.hpp"

#if __has_include(<windows.h>)

#ifndef STRICT
#define STRICT
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <windows.h>
#endif

namespace minimal_serializer {
	namespace generate_string_type_traits {
		/**
		 * Char types outputted as integer in generate_string.
		 */
		template <typename T>
		constexpr bool is_char_as_integer_v = std::is_same_v<T, int8_t> || std::is_same_v<T, uint8_t>;

#ifndef __cpp_char8_t
		/**
		 * Not supported char types in generate_string.
		 */
		template<typename T>
		constexpr bool is_not_supported_char_v = std::is_same_v<T, wchar_t> || std::is_same_v<T, char16_t> || std::is_same_v<T, char32_t>;
#else
		/**
		 * Not supported char types in generate_string.
		 */
		template <typename T>
		constexpr bool is_not_supported_char_v = std::is_same_v<T, wchar_t> || std::is_same_v<T, char8_t> ||
			std::is_same_v<T, char16_t> || std::is_same_v<T, char32_t>;
#endif

		/**
		 * Not supported char array types in generate_string.
		 */
		template <typename T>
		constexpr bool is_not_supported_char_array_v = std::is_same_v<T, wchar_t> || std::is_same_v<T, char16_t> ||
			std::is_same_v<T, char32_t>;
	}

	// return true if generating string from enum type is supported. If this value is false, enum type values are treated as its underlying type values. This depends on nameof C++.
	constexpr bool is_generate_enum_string_supported = nameof::is_nameof_enum_supported;

	inline std::string convert_utf8_to_system_encode(const char* u8_str) {
#if __has_include(<windows.h>)
		// In widows, default strings (char*) are treated as language specific character codes which is not UTF-8, so conversion is required.
		// Convert UTF-8 to wide string (UTF-16)
		constexpr auto get_win32_error_message = [] {
			const auto error_code = GetLastError();
			std::array<char, 1024> error_message{};
			FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, nullptr, error_code, 0u, error_message.data(),
							static_cast<DWORD>(error_message.size()), nullptr);
			std::ostringstream oss;
			oss << error_message.data();
			return oss.str();
		};

		auto const w_str_size = MultiByteToWideChar(CP_UTF8, 0u, u8_str, -1, nullptr, 0u);
		std::vector<wchar_t> w_str(w_str_size, L'\0');
		if (MultiByteToWideChar(CP_UTF8, 0u, u8_str, -1, w_str.data(), static_cast<int>(w_str.size())) == 0) {
			throw std::runtime_error(get_win32_error_message());
		}
		w_str.resize(std::char_traits<wchar_t>::length(w_str.data()));
		w_str.shrink_to_fit();

		// Convert wide string (UTF-16) to system encode.
		std::vector<char> sys_str(w_str.size() * sizeof(wchar_t) + 1, '\0');
		if (WideCharToMultiByte(CP_ACP, 0u, w_str.data(), static_cast<int>(w_str.size()), sys_str.data(),
								static_cast<int>(sys_str.size()), nullptr, nullptr) == 0) {
			throw std::runtime_error(get_win32_error_message());
		}

		sys_str.resize(std::char_traits<char>::length(sys_str.data()));
		sys_str.shrink_to_fit();
		return std::string(sys_str.begin(), sys_str.end());
#else
		// In linux, default strings (char*) are treated as UTF-8, so conversion is not required.
		// How about MacOS X ???
		return std::string(u8_str);
#endif
	}

	template <typename T>
	void generate_string_converter(std::ostringstream& oss, T&& value) {
		using namespace generate_string_type_traits;
		using non_cv_ref_t = std::remove_cv_t<std::remove_reference_t<T>>;
		static_assert(!is_not_supported_char_v<non_cv_ref_t>, "Not supported character type.");
		static_assert(!is_not_supported_char_array_v<non_cv_ref_t>, "Not supported character array type.");

		// char types are recognized as character in ostringstream, so '0' means 'end of string' and '0' in char types will not displayed.
		// To avoid these, cast char types assigned to uint8_t and int8_t to int before pass them to ostringstream
		if constexpr (is_char_as_integer_v<non_cv_ref_t>) {
			if constexpr (std::is_signed_v<non_cv_ref_t>) {
				oss << static_cast<int32_t>(value);
			}
			else {
				oss << static_cast<uint32_t>(value);
			}
		}
#if __cpp_char8_t
		// char8_t support
		else if constexpr (std::is_pointer_v<non_cv_ref_t> && std::is_same_v<
			std::remove_const_t<std::remove_pointer_t<non_cv_ref_t>>, char8_t>) {
			oss << convert_utf8_to_system_encode(reinterpret_cast<char*>(const_cast<char8_t*>(value)));
		}
		else if constexpr (std::is_array_v<non_cv_ref_t> && std::is_same_v<
			std::remove_extent_t<non_cv_ref_t>, char8_t>) {
			oss << convert_utf8_to_system_encode(reinterpret_cast<char*>(const_cast<char8_t*>(&value[0])));
		}
		else if constexpr (std::is_same_v<non_cv_ref_t, std::u8string>) {
			oss << convert_utf8_to_system_encode(reinterpret_cast<const char*>(const_cast<char8_t*>(value.c_str())));
		}
#endif
		else if constexpr (std::is_enum_v<non_cv_ref_t>) {
			if constexpr (is_generate_enum_string_supported) {
				oss << nameof::nameof_enum(value);
			}
			else {
				oss << static_cast<std::underlying_type_t<non_cv_ref_t>>(value);
			}
		}
		else {
			oss << value;
		}
	}

	template <>
	inline void generate_string_converter(std::ostringstream& oss, const std::type_info& value) {
		oss << value.name();
	}

	inline void generate_string_impl(std::ostringstream&) {}

	template <typename First, typename ... Rest>
	void generate_string_impl(std::ostringstream& oss, First&& first, Rest&& ... rest) {
		generate_string_converter(oss, std::forward<First>(first));
		generate_string_impl(oss, std::forward<Rest>(rest)...);
	}

	/**
	 * Convert parameters to string and concatenate them to one string. This is thread safe.
	 * @note Main use case of this function is log string generation which is outputted to std::ostream. So we dont implement the version to return std::u8string because std::u8string is not supported as std::ostream input in current C++ version (C++20).
	 */
	template <typename ... Params>
	std::string generate_string(Params&& ... params) {
		std::ostringstream oss;
		oss << std::boolalpha;
		generate_string_impl(oss, std::forward<Params>(params)...);
		return oss.str();
	}
}
