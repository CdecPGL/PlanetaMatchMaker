using System;
using System.Net;
using System.Text.RegularExpressions;

namespace PlanetaGameLabo.MatchMaker
{
    /// <summary>
    /// Immutable PMMS server address.
    /// IPv4, IPv6 or host name is valid.
    /// </summary>
    public sealed class MatchMakerServerAddress : IEquatable<MatchMakerServerAddress>
    {
        public MatchMakerServerAddress(string value)
        {
            if (value == null)
            {
                throw new ArgumentNullException(nameof(value));
            }

            if (!TryNormalize(value, out var normalizedValue))
            {
                throw new ArgumentException("IPv4, IPv6 or host name is available.", nameof(value));
            }

            Value = normalizedValue;
        }

        public string Value { get; }

        public static MatchMakerServerAddress Parse(string value)
        {
            return new MatchMakerServerAddress(value);
        }

        public static bool TryParse(string value, out MatchMakerServerAddress address)
        {
            if (!TryNormalize(value, out var normalizedValue))
            {
                address = null;
                return false;
            }

            address = new MatchMakerServerAddress(normalizedValue);
            return true;
        }

        public bool Equals(MatchMakerServerAddress other)
        {
            return other != null && string.Equals(Value, other.Value, StringComparison.OrdinalIgnoreCase);
        }

        public override bool Equals(object obj)
        {
            return Equals(obj as MatchMakerServerAddress);
        }

        public override int GetHashCode()
        {
            return StringComparer.OrdinalIgnoreCase.GetHashCode(Value);
        }

        public override string ToString()
        {
            return Value;
        }

        internal static bool IsValid(string value)
        {
            return TryNormalize(value, out _);
        }

        private static bool TryNormalize(string value, out string normalizedValue)
        {
            normalizedValue = null;

            if (string.IsNullOrEmpty(value))
            {
                return false;
            }

            if (IPAddress.TryParse(value, out _))
            {
                normalizedValue = value;
                return true;
            }

            if (!HostNameRegex.IsMatch(value))
            {
                return false;
            }

            normalizedValue = value;
            return true;
        }

        private static readonly Regex HostNameRegex =
            new Regex(@"^[A-Za-z0-9_-]+(\.[A-Za-z0-9_-]+)*$", RegexOptions.CultureInvariant);
    }
}
