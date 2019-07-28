using System;

namespace PlanetaGameLabo.MatchMaker {
    [Serializable]
    public sealed class Datetime {
        public Datetime(int year, int month, int day) : this(year, month, day, 0, 0, 0) { }

        public Datetime(int year, int month, int day, int hour, int minute, int second) {
            var datetime = new DateTime(year, month, day, hour, minute, second);
            _unixTime = new DateTimeOffset(datetime.Ticks, new TimeSpan(0, 0, 0)).ToUnixTimeSeconds();
        }

        public int year => GetUtcDatetime().Year;

        public int month => GetUtcDatetime().Month;

        public int day => GetUtcDatetime().Day;

        public int hour => GetUtcDatetime().Hour;

        public int minute => GetUtcDatetime().Minute;

        public int second => GetUtcDatetime().Second;

        public static Datetime Now() {
            var now = DateTime.UtcNow;
            return new Datetime(now.Year, now.Month, now.Day, now.Hour, now.Minute, now.Second);
        }

        private long _unixTime;

        private DateTime GetUtcDatetime() {
            return DateTimeOffset.FromUnixTimeSeconds(_unixTime).UtcDateTime;
        }
    }
}