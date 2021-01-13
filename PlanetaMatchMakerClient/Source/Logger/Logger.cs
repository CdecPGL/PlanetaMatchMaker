using System;
using System.Diagnostics;
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

    public abstract class LoggerBase : ILogger
    {
        public bool Enabled { get; set; } = true;
        public LogLevel LogLevel { get; } = LogLevel.Info;

        public void Log(LogLevel level, string message)
        {
            if (!Enabled || level < LogLevel)
            {
                return;
            }

            LogImpl(level, FormatLog(level, message));
        }

        protected abstract void LogImpl(LogLevel level, string formattedMessage);

        private static string FormatLog(LogLevel level, string message)
        {
            return $"[{DateTime.UtcNow:yyyy-MM-dd HH:mm:ss:ffffff} UTC] {level}: {message}";
        }
    }

    public sealed class StreamLogger : LoggerBase
    {
        public StreamLogger(StreamWriter standardStreamWriter, StreamWriter errorStreamWriter)
        {
            Debug.Assert(standardStreamWriter != null, nameof(this.standardStreamWriter) + " != null");
            standardStreamWriter.AutoFlush = true;
            this.standardStreamWriter = standardStreamWriter;
            this.errorStreamWriter = errorStreamWriter;
        }

        protected override void LogImpl(LogLevel level, string formatterMessage)
        {
            switch (level)
            {
                case LogLevel.Debug:
                case LogLevel.Info:
                    standardStreamWriter.WriteLine(formatterMessage);
                    break;
                case LogLevel.Warning:
                case LogLevel.Error:
                    errorStreamWriter.WriteLine(formatterMessage);
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