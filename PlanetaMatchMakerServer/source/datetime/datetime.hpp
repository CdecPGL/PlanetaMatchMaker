#pragma once

#include <cstdint>

namespace pgl {
	// 8 bytes
	struct datetime final {
		datetime() = default;
		datetime(int year, int month, int day);
		datetime(int year, int month, int day, int hour, int minuit, int second);

		[[nodiscard]] int get_year() const;

		[[nodiscard]] int get_month() const;

		[[nodiscard]] int get_day() const;

		[[nodiscard]] int get_hour() const;

		[[nodiscard]] int get_minuit() const;

		[[nodiscard]] int get_second() const;

	private:
		uint64_t date_{};

		[[nodiscard]] int get_from_date(int start_bit, int bit_count) const;
	};
}
