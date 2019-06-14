#include <chrono>
#include <ctime>

#include <boost/date_time/posix_time/posix_time.hpp>

#include "date/date.h"

#include "datetime.hpp"

using namespace std;

namespace pgl {
	constexpr int second_start_bit = 0;
	constexpr int second_bit_count = 6;
	constexpr int minuit_start_bit = second_start_bit + second_bit_count;
	constexpr int minuit_bit_count = 6;
	constexpr int hour_start_bit = minuit_start_bit + minuit_bit_count;
	constexpr int hour_bit_count = 5;
	constexpr int day_start_bit = hour_start_bit + hour_bit_count;
	constexpr int day_bit_count = 5;
	constexpr int month_start_bit = day_start_bit + day_bit_count;
	constexpr int month_bit_count = 4;
	constexpr int year_start_bit = month_start_bit + month_bit_count;
	constexpr int year_bit_count = 31;

	uint64_t get_located_data(const uint64_t value, const int start_bit, const int bit_count) {
		return (value & ((1ll << bit_count) - 1)) << start_bit;
	}

	datetime::datetime(const int year, const int month, const int day): datetime(year, month, day, 0, 0, 0) {}

	datetime::datetime(const int year, const int month, const int day, const int hour, const int minuit,
		const int second): data_(
		get_located_data(year, year_start_bit, year_bit_count) &
		get_located_data(month, month_start_bit, month_bit_count) &
		get_located_data(day, day_start_bit, day_bit_count) &
		get_located_data(hour, hour_start_bit, hour_bit_count) &
		get_located_data(minuit, minuit_start_bit, minuit_bit_count) &
		get_located_data(second, second_start_bit, second_bit_count)
	) {}

	int datetime::year() const {
		return get_from_date(year_start_bit, year_bit_count);
	}

	int datetime::month() const {
		return get_from_date(month_start_bit, month_bit_count);
	}

	int datetime::day() const {
		return get_from_date(day_start_bit, day_bit_count);
	}

	int datetime::hour() const {
		return get_from_date(hour_start_bit, hour_bit_count);
	}

	int datetime::minuit() const {
		return get_from_date(minuit_start_bit, minuit_bit_count);
	}

	int datetime::second() const {
		return get_from_date(second_start_bit, second_bit_count);
	}

	size_t datetime::get_hash() const {
		return boost::hash_value(data_);
	}

	bool datetime::operator<(const datetime& other) const {
		return data_ < other.data_;
	}

	bool datetime::operator==(const datetime& other) const {
		return data_ == other.data_;
	}

	datetime datetime::now() {
		const auto date_time = boost::posix_time::second_clock::universal_time();
		const auto date = date_time.date();
		const auto time = date_time.time_of_day();
		return datetime(date.year(), date.month(), date.day(), static_cast<int>(time.hours()),
			static_cast<int>(time.minutes()), static_cast<int>(time.seconds()));
	}

	int datetime::get_from_date(const int start_bit, const int bit_count) const {
		return static_cast<int>(data_ << start_bit) & ((1 << bit_count) - 1);
	}

	string get_now_time_string() {
		const auto now_time = std::chrono::system_clock::now();
		return date::format("%F %T %Z", now_time);
	}
}
