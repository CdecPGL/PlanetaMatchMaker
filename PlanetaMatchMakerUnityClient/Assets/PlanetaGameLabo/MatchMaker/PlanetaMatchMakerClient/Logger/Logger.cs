using System;
using System.IO;

namespace PlanetaGameLabo.MatchMaker
{
    public enum LogLevel
    {
        Debug,
        Info,
        Warning,
        Error
    }

    public interface ILogger
    {
        bool Enabled { get; set; }

        LogLevel LogLevel { get; }

        void Log(LogLevel level, string message);
    }

    public static class LoggerExtensions
    {
        public static string FormatLog(LogLevel level, string message)
        {
            return $"[{DateTime.UtcNow:yyyy-MM-dd HH:mm:ss:ffffff} UTF] {level}: {message}";
        }
    }

    public sealed class StreamLogger : ILogger
    {
        public StreamLogger(StreamWriter standardStreamWriter, StreamWriter errorStreamWriter)
        {
            standardStreamWriter.AutoFlush = true;
            this.standardStreamWriter = standardStreamWriter;
            this.errorStreamWriter = errorStreamWriter;
        }

        public bool Enabled { get; set; } = true;

        public LogLevel LogLevel { get; } = LogLevel.Info;

        public void Log(LogLevel level, string message)
        {
            if (!Enabled || level < LogLevel)
            {
                return;
            }

            switch (level)
            {
                case LogLevel.Debug:
                case LogLevel.Info:
                    standardStreamWriter.WriteLine(LoggerExtensions.FormatLog(level, message));
                    break;
                case LogLevel.Warning:
                case LogLevel.Error:
                    errorStreamWriter.WriteLine(LoggerExtensions.FormatLog(level, message));
                    break;
                default:
                    throw new ArgumentOutOfRangeException(nameof(level), level, null);
            }
        }

        public static ILogger CreateStandardOutputLogger()
        {
            return new StreamLogger(new StreamWriter(Console.OpenStandardOutput(), Console.OutputEncoding),
                new StreamWriter(Console.OpenStandardError(), Console.OutputEncoding));
        }

        public static ILogger CreateNullLogger()
        {
            return new StreamLogger(new StreamWriter(Stream.Null), new StreamWriter(Stream.Null));
        }

        private readonly StreamWriter standardStreamWriter;
        private readonly StreamWriter errorStreamWriter;
    }
}