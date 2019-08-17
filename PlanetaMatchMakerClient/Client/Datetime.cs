using System;
using System.Runtime.InteropServices;

namespace PlanetaGameLabo.MatchMaker
{
    [Serializable, StructLayout(LayoutKind.Sequential)]
    public sealed class Datetime
    {
        public Datetime(int year, int month, int day) : this(year, month, day, 0, 0, 0)
        {
        }

        public Datetime(int year, int month, int day, int hour, int minute, int second)
        {
            var datetime = new DateTime(year, month, day, hour, minute, second);
            unixTime = new DateTimeOffset(datetime.Ticks, new TimeSpan(0, 0, 0)).ToUnixTimeSeconds();
        }

        public int Year => GetUtcDatetime().Year;

        public int Month => GetUtcDatetime().Month;

        public int Day => GetUtcDatetime().Day;

        public int Hour => GetUtcDatetime().Hour;

        public int Minute => GetUtcDatetime().Minute;

        public int Second => GetUtcDatetime().Second;

        public static Datetime Now()
        {
            var now = DateTime.UtcNow;
            return new Datetime(now.Year, now.Month, now.Day, now.Hour, now.Minute, now.Second);
        }

        private readonly long unixTime;

        private DateTime GetUtcDatetime()
        {
            return DateTimeOffset.FromUnixTimeSeconds(unixTime).UtcDateTime;
        }
    }
}