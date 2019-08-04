#include <ctime>

#include <boost/date_time/posix_time/posix_time.hpp>

#include "datetime.hpp"

using namespace std;

namespace pgl {
	int64_t get_unix_time(const int year, const int month, const int day, const int hours, const int minutes,
		const int seconds) {
		const boost::posix_time::ptime epoch(boost::gregorian::date(1970, 1, 1));
		const boost::posix_time::ptime date_time(boost::gregorian::date(static_cast<const unsigned short>(year),
				static_cast<const unsigned short>(month), static_cast<const unsigned short>(day)),
			boost::posix_time::time_duration(hours, minutes, seconds));
		return (date_time - epoch).total_seconds();
	}

	void get_boost_ptime_from_unix_time(const int64_t unix_time, boost::posix_time::ptime& ptime) {
		ptime = boost::posix_time::ptime(boost::gregorian::date(1970, 1, 1), boost::posix_time::seconds(unix_time));
	}

	datetime::datetime(const int year, const int month, const int day): datetime(year, month, day, 0, 0, 0) {}

	datetime::datetime(const int year, const int month, const int day, const int hour, const int minute,
		const int second): unix_time_(get_unix_time(year, month, day, hour, minute, second)) {}

	int datetime::year() const {
		boost::posix_time::ptime ptime;
		get_boost_ptime_from_unix_time(unix_time_, ptime);
		return ptime.date().year();
	}

	int datetime::month() const {
		boost::posix_time::ptime ptime;
		get_boost_ptime_from_unix_time(unix_time_, ptime);
		return ptime.date().month();
	}

	int datetime::day() const {
		boost::posix_time::ptime ptime;
		get_boost_ptime_from_unix_time(unix_time_, ptime);
		return ptime.date().day();
	}

	int datetime::hour() const {
		boost::posix_time::ptime ptime;
		get_boost_ptime_from_unix_time(unix_time_, ptime);
		return static_cast<int>(ptime.time_of_day().hours());
	}

	int datetime::minute() const {
		boost::posix_time::ptime ptime;
		get_boost_ptime_from_unix_time(unix_time_, ptime);
		return static_cast<int>(ptime.time_of_day().minutes());
	}

	int datetime::second() const {
		boost::posix_time::ptime ptime;
		get_boost_ptime_from_unix_time(unix_time_, ptime);
		return static_cast<int>(ptime.time_of_day().seconds());
	}

	size_t datetime::get_hash() const {
		return boost::hash_value(unix_time_);
	}

	bool datetime::operator<(const datetime& other) const {
		return unix_time_ < other.unix_time_;
	}

	bool datetime::operator==(const datetime& other) const {
		return unix_time_ == other.unix_time_;
	}

	datetime datetime::now() {
		const auto date_time = boost::posix_time::second_clock::universal_time();
		const auto date = date_time.date();
		const auto time = date_time.time_of_day();
		return datetime(date.year(), date.month(), date.day(), static_cast<int>(time.hours()),
			static_cast<int>(time.minutes()), static_cast<int>(time.seconds()));
	}

	string get_now_datetime_string() {
		const auto date_time = boost::posix_time::microsec_clock::universal_time();
		ostringstream oss;
		// deleting of std::locale::facet mey not be necessary.
		auto facet = new boost::posix_time::time_facet();
		facet->format("%Y-%m-%d %H:%M:%S.%f UTC");
		oss.imbue(locale(locale::classic(), facet));
		oss << date_time;
		return oss.str();
	}
}
