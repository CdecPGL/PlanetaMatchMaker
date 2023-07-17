/*
The MIT License (MIT)

Copyright (c) 2019 Cdec

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#pragma once

#include <array>
#include <string>

#include <boost/operators.hpp>
#include <boost/functional/hash.hpp>

#include "string_utility.hpp"

namespace minimal_serializer {
	/**
	 * @brief A template of fixed length string which is trivial type. This class holds characters as uint8_t instead of char.
	 * @tparam String A string type of STL.
	 * @tparam Length A max length of string.
	 */
	template <typename String, size_t Length>
	class fixed_string_base : boost::less_than_comparable<fixed_string_base<String, Length>>,
							boost::equality_comparable<fixed_string_base<String, Length>> {
		using char_t = typename String::value_type;
		static_assert(sizeof(char_t) == sizeof(uint8_t),
					"The type of character must have same size with uint8_t (1byte).");

	public:
		constexpr fixed_string_base() = default;
		constexpr fixed_string_base(const fixed_string_base& other) = default;
		constexpr fixed_string_base(fixed_string_base&& other) = default;

		/**
		 * @brief A constructor from raw string.
		 * @param c_str A raw string by char array.
		 */
		fixed_string_base(const char_t* c_str) {
			const auto str_length = get_c_string_length(c_str);
			if (str_length > Length) {
				const auto message = generate_string("The length of string (", str_length, " exceeds defined length (",
													Length, ")");
				throw std::out_of_range(message);
			}

			std::memcpy(data_.data(), c_str, str_length);
			for (auto i = str_length; i < Length; ++i) data_[i] = 0;
		}

		/**
		 * @brief A constructor from stl string.
		 * @param str A string by stl string.
		 */
		constexpr fixed_string_base(const String& str) : fixed_string_base(str.c_str()) { }

		~fixed_string_base() = default;

		constexpr fixed_string_base& operator=(const fixed_string_base& other) = default;
		constexpr fixed_string_base& operator=(fixed_string_base&& other) = default;

		constexpr uint8_t operator[](size_t idx) const {
			return data_[idx];
		}

		constexpr uint8_t& operator[](size_t idx) {
			return data_[idx];
		}

		[[nodiscard]] constexpr auto begin() const {
			return data_.begin();
		}

		[[nodiscard]] constexpr auto end() const {
			return data_.end();
		}

		constexpr bool operator<(const fixed_string_base& other) const {
			return to_string() < other.to_string();
		}

		constexpr bool operator==(const fixed_string_base& other) const {
			return data_ == other.data_;
		}

		[[nodiscard]] constexpr uint8_t at(size_t idx) const {
			return data_.at(idx);
		}

		[[nodiscard]] constexpr uint8_t& at(size_t idx) {
			return data_.at(idx);
		}

		/**
		 * @brief Get the actual size of the string without terminal character "\0". This is same as length().
		 * @return The actual size of the string.
		 */
		[[nodiscard]] constexpr size_t size() const {
			return data_[Length - 1] ? Length : get_c_string_length(reinterpret_cast<const char_t*>(data_.data()));
		}

		/**
		 * @brief Get the actual size of the string without terminal character "\0". This is same as size().
		 * @return The actual size of the string.
		 */
		[[nodiscard]] constexpr size_t length() const {
			return size();
		}

		/**
		 * @brief Get the max size of this string class.
		 * @return The max size of this string class.
		 */
		[[nodiscard]] constexpr size_t max_size() const {
			return Length;
		}

		[[nodiscard]] String to_string() const {
			if (data_[Length - 1]) {
				char_t c_str[Length + 1];
				std::memcpy(c_str, data_.data(), Length);
				c_str[Length] = 0;
				return String(c_str);
			}

			return String(reinterpret_cast<const char_t*>(data_.data()));
		}

	private:
		std::array<uint8_t, Length> data_;

		[[nodiscard]] constexpr static size_t get_c_string_length(const char_t* c_str) {
			return *c_str ? 1 + get_c_string_length(c_str + 1) : 0;
		}

	public:
		using serialize_targets = serialize_target_container<&fixed_string_base::data_>;
	};

	/**
	 *Enable std::ostream output.
	 */
	template <typename String, size_t Length>
	std::ostream& operator <<(std::ostream& os, const fixed_string_base<String, Length>& fixed_string) {
#ifndef __cpp_char8_t
		os << fixed_string.to_string().c_str();
		return os;
#else
		using char_t = typename String::value_type;
		if constexpr (std::is_same_v<char_t, char8_t>) {
			os << convert_utf8_to_system_encode(
				reinterpret_cast<char*>(const_cast<char_t*>(fixed_string.to_string().c_str())));
			return os;
		}
		else {
			os << fixed_string.to_string().c_str();
			return os;
		}
#endif
	}

#ifndef __cpp_char8_t
	/**
	 * @brief A fixed length string which is trivial type. This class holds characters as uint8_t instead of char.
	 * @tparam Length A max length of string.
	 */
	template <size_t Length>
	using fixed_string = fixed_string_base<std::string, Length>;
#else
	/**
	 * @brief A fixed length string which is trivial type. This class holds characters as uint8_t instead of char.
	 * @tparam Length A max length of string.
	 * @deprecated Recommend to use fixed_u8string instead.
	 */
	template <size_t Length>
	using fixed_string [[deprecated("Recommend to use fixed_u8string instead.")]] = fixed_string_base<
		std::string, Length>;

	/**
	 * @brief A fixed length UTF-8 string which is trivial type. This class holds characters as uint8_t instead of char.
	 * @tparam Length A max length of string.
	 */
	template <size_t Length>
	using fixed_u8string = fixed_string_base<std::u8string, Length>;
#endif
}

namespace minimal_serializer {
	template <typename String, size_t Length>
	size_t hash_value(const fixed_string_base<String, Length>& fixed_string) {
		return boost::hash_range(fixed_string.begin(), fixed_string.end());
	}
}

namespace std {
	template <typename String, size_t Length>
	struct hash<minimal_serializer::fixed_string_base<String, Length>> {
		size_t operator()(const minimal_serializer::fixed_string_base<String, Length>& fixed_string) const noexcept {
			return boost::hash_range(fixed_string.begin(), fixed_string.end());
		}
	};
}
