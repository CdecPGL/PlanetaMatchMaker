#pragma once

#include <cstddef>

namespace pgl {
	template <typename FixedU8String>
	[[nodiscard]] bool is_canonical_fixed_u8string(const FixedU8String& value) {
		std::size_t length = 0;
		while (length < value.max_size() && value[length] != 0) { ++length; }
		for (auto i = length; i < value.max_size(); ++i) {
			if (value[i] != 0) { return false; }
		}

		const auto is_continuation = [&value, length](const std::size_t index) {
			return index < length && (value[index] & 0xc0) == 0x80;
		};
		for (std::size_t i = 0; i < length;) {
			const auto first = value[i];
			if (first <= 0x7f) {
				++i;
				continue;
			}
			if (first >= 0xc2 && first <= 0xdf && is_continuation(i + 1)) {
				i += 2;
				continue;
			}
			if (first == 0xe0 && i + 2 < length && value[i + 1] >= 0xa0 && value[i + 1] <= 0xbf &&
				is_continuation(i + 2)) {
				i += 3;
				continue;
			}
			if (((first >= 0xe1 && first <= 0xec) || (first >= 0xee && first <= 0xef)) &&
				is_continuation(i + 1) && is_continuation(i + 2)) {
				i += 3;
				continue;
			}
			if (first == 0xed && i + 2 < length && value[i + 1] >= 0x80 && value[i + 1] <= 0x9f &&
				is_continuation(i + 2)) {
				i += 3;
				continue;
			}
			if (first == 0xf0 && i + 3 < length && value[i + 1] >= 0x90 && value[i + 1] <= 0xbf &&
				is_continuation(i + 2) && is_continuation(i + 3)) {
				i += 4;
				continue;
			}
			if (first >= 0xf1 && first <= 0xf3 && is_continuation(i + 1) && is_continuation(i + 2) &&
				is_continuation(i + 3)) {
				i += 4;
				continue;
			}
			if (first == 0xf4 && i + 3 < length && value[i + 1] >= 0x80 && value[i + 1] <= 0x8f &&
				is_continuation(i + 2) && is_continuation(i + 3)) {
				i += 4;
				continue;
			}
			return false;
		}
		return true;
	}
}
