#pragma once

#include <cstdint>
#include <string>

#include <boost/functional/hash.hpp>

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

		[[nodiscard]] size_t get_hash() const {
			return boost::hash_value(data_);
		}

		static datetime now();

	private:
		uint64_t data_{};

		[[nodiscard]] int get_from_date(int start_bit, int bit_count) const;
	};

	std::string get_time_string();
}

namespace boost {
	inline size_t hash_value(const pgl::datetime& datetime) {
		return datetime.get_hash();
	}
}

namespace std {
	template <>
	struct hash<pgl::datetime> {
		size_t operator()(const pgl::datetime& datetime) const noexcept {
			return boost::hash_value(datetime);
		}
	};
}
