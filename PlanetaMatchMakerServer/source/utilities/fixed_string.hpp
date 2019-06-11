#pragma once

#include <array>
#include <string>
#include <cassert>

namespace pgl {
	template <size_t Length>
	class fixed_string final {
	public:
		explicit constexpr fixed_string(const char* c_str) {
			auto str_length = get_c_string_length(c_str);
			static_assert(str_length <= Length, "The length of string exceeds limit.");
			assert(str_length <= Length, "The length of string exceeds limit.");
			data_.fill(0);
			std::memcpy(data_.data(), c_str, str_length);
		}

		explicit constexpr fixed_string(const std::string& str) : fixed_string(str.c_str()) { }

		[[nodiscard]] constexpr size_t length() const {
			return get_c_string_length(data_.data());
		}

		[[nodiscard]] constexpr std::string to_string() const {
			return std::string(data_.data());
		}

		constexpr static size_t get_c_string_length(const char* c_str) {
			return *c_str ? 1 + get_c_string_length(c_str + 1) : 0;
		}

	private:
		std::array<char, Length> data_;
	};

	template <size_t Length>
	std::ostream& operator <<(std::ostream& os, const fixed_string<Length>& fixed_string) {
		os << fixed_string.to_string();
		return os;
	}
}
