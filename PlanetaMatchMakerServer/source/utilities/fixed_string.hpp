#pragma once

#include <array>
#include <string>

#include <boost/operators.hpp>

#include "serialize/serializer.hpp"
#include "string_utility.hpp"

namespace pgl {
	// Fixed length string which is trivial type. THis class holds characters as uint8_t instead of char.
	template <size_t Length>
	class fixed_string final : boost::less_than_comparable<fixed_string<Length>>,
		boost::equality_comparable<fixed_string<Length>> {
	public:
		constexpr fixed_string() = default;
		constexpr fixed_string(const fixed_string& other) = default;
		constexpr fixed_string(fixed_string&& other) = default;

		fixed_string(const char* c_str) {
			const auto str_length = get_c_string_length(c_str);
			if (str_length > Length) {
				const auto message = generate_string("The length of string (", str_length, " exceeds defined length (",
					Length, ")");
				throw std::out_of_range(message);
			}

			std::memcpy(data_.data(), c_str, str_length);
			for (auto i = str_length; i < Length; ++i) data_[i] = 0;
		}

		constexpr fixed_string(const std::string& str) : fixed_string(str.c_str()) { }

		~fixed_string() = default;

		constexpr fixed_string& operator=(const fixed_string& other) = default;
		constexpr fixed_string& operator=(fixed_string&& other) = default;

		constexpr uint8_t operator[](size_t idx) const {
			return data_[idx];
		}

		constexpr uint8_t& operator[](size_t idx) {
			return data_[idx];
		}

		constexpr bool operator<(const fixed_string& other) const {
			return to_string() < other.to_string();
		}

		constexpr bool operator==(const fixed_string& other) const {
			return data_ == other.data_;
		}

		[[nodiscard]] constexpr uint8_t at(size_t idx) const {
			return data_.at(idx);
		}

		constexpr uint8_t& at(size_t idx) {
			return data_.at(idx);
		}

		[[nodiscard]] constexpr size_t size() const {
			return data_[Length - 1] ? Length : get_c_string_length(reinterpret_cast<const char*>(data_.data()));
		}

		[[nodiscard]] constexpr std::string to_string() const {
			if (data_[Length - 1]) {
				char c_str[Length + 1];
				std::memcpy(c_str, data_.data(), Length);
				c_str[Length] = 0;
				return std::string(c_str);
			}

			return std::string(reinterpret_cast<const char*>(data_.data()));
		}

		constexpr static size_t get_c_string_length(const char* c_str) {
			return *c_str ? 1 + get_c_string_length(c_str + 1) : 0;
		}

		void on_serialize(serializer& serializer) {
			serializer += data_;
		}

	private:
		std::array<uint8_t, Length> data_;
	};

	template <size_t Length>
	std::ostream& operator <<(std::ostream& os, const fixed_string<Length>& fixed_string) {
		os << fixed_string.to_string();
		return os;
	}
}