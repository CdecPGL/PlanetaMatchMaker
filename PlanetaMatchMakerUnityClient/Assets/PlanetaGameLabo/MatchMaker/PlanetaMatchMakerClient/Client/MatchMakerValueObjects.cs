using System;
using System.Globalization;
using System.Linq;
using System.Net;
using System.Text;
using System.Text.RegularExpressions;
using CdecPGL.MinimalSerializer;

namespace PlanetaGameLabo.MatchMaker
{
    public sealed class MatchMakerGameId : IEquatable<MatchMakerGameId>
    {
        public MatchMakerGameId(string value)
        {
            if (value == null)
            {
                throw new ArgumentNullException(nameof(value));
            }

            if (!IsValid(value))
            {
                throw new ArgumentException(
                    $"The length of Game ID must be in range [{1}, {ClientConstants.GameIdLength}].",
                    nameof(value));
            }

            Value = value;
        }

        public string Value { get; }

        public static MatchMakerGameId Parse(string value)
        {
            return new MatchMakerGameId(value);
        }

        public static bool TryParse(string value, out MatchMakerGameId gameId)
        {
            if (!IsValid(value))
            {
                gameId = null;
                return false;
            }

            gameId = new MatchMakerGameId(value);
            return true;
        }

        public bool Equals(MatchMakerGameId other)
        {
            return other != null && string.Equals(Value, other.Value, StringComparison.Ordinal);
        }

        public override bool Equals(object obj)
        {
            return Equals(obj as MatchMakerGameId);
        }

        public override int GetHashCode()
        {
            return StringComparer.Ordinal.GetHashCode(Value);
        }

        public override string ToString()
        {
            return Value;
        }

        internal static bool IsValid(string value)
        {
            return !string.IsNullOrEmpty(value) && value.Length <= ClientConstants.GameIdLength;
        }
    }

    public sealed class MatchMakerGameVersion : IEquatable<MatchMakerGameVersion>
    {
        public MatchMakerGameVersion(string value)
        {
            if (value == null)
            {
                throw new ArgumentNullException(nameof(value));
            }

            if (!IsValid(value))
            {
                throw new ArgumentException(
                    $"The length of Game Version must be in range [{0}, {ClientConstants.GameVersionLength}].",
                    nameof(value));
            }

            Value = value;
        }

        public static MatchMakerGameVersion Empty { get; } = new MatchMakerGameVersion("");

        public string Value { get; }

        public static MatchMakerGameVersion Parse(string value)
        {
            return new MatchMakerGameVersion(value);
        }

        public static bool TryParse(string value, out MatchMakerGameVersion gameVersion)
        {
            if (!IsValid(value))
            {
                gameVersion = null;
                return false;
            }

            gameVersion = new MatchMakerGameVersion(value);
            return true;
        }

        public bool Equals(MatchMakerGameVersion other)
        {
            return other != null && string.Equals(Value, other.Value, StringComparison.Ordinal);
        }

        public override bool Equals(object obj)
        {
            return Equals(obj as MatchMakerGameVersion);
        }

        public override int GetHashCode()
        {
            return StringComparer.Ordinal.GetHashCode(Value);
        }

        public override string ToString()
        {
            return Value;
        }

        internal static bool IsValid(string value)
        {
            return value != null && value.Length <= ClientConstants.GameVersionLength;
        }
    }

    /// <summary>
    /// Immutable host or IP address for PMMS connections.
    /// IPv4, IPv6 or host name is valid.
    /// </summary>
    public class MatchMakerAddress : IEquatable<MatchMakerAddress>
    {
        public MatchMakerAddress(string value)
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

        public static MatchMakerAddress Parse(string value)
        {
            return new MatchMakerAddress(value);
        }

        public static bool TryParse(string value, out MatchMakerAddress address)
        {
            if (!TryNormalize(value, out var normalizedValue))
            {
                address = null;
                return false;
            }

            address = new MatchMakerAddress(normalizedValue);
            return true;
        }

        public bool Equals(MatchMakerAddress other)
        {
            return other != null && string.Equals(Value, other.Value, StringComparison.OrdinalIgnoreCase);
        }

        public override bool Equals(object obj)
        {
            return Equals(obj as MatchMakerAddress);
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

    public sealed class MatchMakerServerPort : IEquatable<MatchMakerServerPort>
    {
        public MatchMakerServerPort(ushort value)
        {
            if (!IsValid(value))
            {
                throw new ArgumentException("0 is not available.", nameof(value));
            }

            Value = value;
        }

        public ushort Value { get; }

        public static MatchMakerServerPort Parse(ushort value)
        {
            return new MatchMakerServerPort(value);
        }

        public static bool TryParse(ushort value, out MatchMakerServerPort port)
        {
            if (!IsValid(value))
            {
                port = null;
                return false;
            }

            port = new MatchMakerServerPort(value);
            return true;
        }

        public bool Equals(MatchMakerServerPort other)
        {
            return other != null && Value == other.Value;
        }

        public override bool Equals(object obj)
        {
            return Equals(obj as MatchMakerServerPort);
        }

        public override int GetHashCode()
        {
            return Value.GetHashCode();
        }

        public override string ToString()
        {
            return Value.ToString(CultureInfo.InvariantCulture);
        }

        internal static bool IsValid(ushort value)
        {
            return value != 0;
        }
    }

    public sealed class MatchMakerPlayerName : IEquatable<MatchMakerPlayerName>
    {
        public MatchMakerPlayerName(string value)
        {
            if (value == null)
            {
                throw new ArgumentNullException(nameof(value));
            }

            if (!IsValid(value))
            {
                throw new ArgumentException(
                    $"null string or string whose length is more than {ClientConstants.PlayerNameLength} is not available.",
                    nameof(value));
            }

            Value = value;
        }

        public string Value { get; }

        public static MatchMakerPlayerName Parse(string value)
        {
            return new MatchMakerPlayerName(value);
        }

        public static bool TryParse(string value, out MatchMakerPlayerName playerName)
        {
            if (!IsValid(value))
            {
                playerName = null;
                return false;
            }

            playerName = new MatchMakerPlayerName(value);
            return true;
        }

        public bool Equals(MatchMakerPlayerName other)
        {
            return other != null && string.Equals(Value, other.Value, StringComparison.Ordinal);
        }

        public override bool Equals(object obj)
        {
            return Equals(obj as MatchMakerPlayerName);
        }

        public override int GetHashCode()
        {
            return StringComparer.Ordinal.GetHashCode(Value);
        }

        public override string ToString()
        {
            return Value;
        }

        internal static bool IsValid(string value)
        {
            return !string.IsNullOrWhiteSpace(value) &&
                   Encoding.UTF8.GetByteCount(value) <= ClientConstants.PlayerNameLength;
        }
    }

    public sealed class MatchMakerGameHostPort : IEquatable<MatchMakerGameHostPort>
    {
        public MatchMakerGameHostPort(ushort value)
        {
            if (!IsValid(value))
            {
                throw new ArgumentException("Dynamic/private port is available.", nameof(value));
            }

            Value = value;
        }

        public ushort Value { get; }

        public static MatchMakerGameHostPort Parse(ushort value)
        {
            return new MatchMakerGameHostPort(value);
        }

        public static bool TryParse(ushort value, out MatchMakerGameHostPort port)
        {
            if (!IsValid(value))
            {
                port = null;
                return false;
            }

            port = new MatchMakerGameHostPort(value);
            return true;
        }

        public bool Equals(MatchMakerGameHostPort other)
        {
            return other != null && Value == other.Value;
        }

        public override bool Equals(object obj)
        {
            return Equals(obj as MatchMakerGameHostPort);
        }

        public override int GetHashCode()
        {
            return Value.GetHashCode();
        }

        public override string ToString()
        {
            return Value.ToString(CultureInfo.InvariantCulture);
        }

        internal static bool IsValid(ushort value)
        {
            return 49152 <= value && value <= 65535;
        }
    }

    public sealed class MatchMakerGameHostExternalId : IEquatable<MatchMakerGameHostExternalId>
    {
        public MatchMakerGameHostExternalId(byte[] value)
        {
            if (value == null)
            {
                throw new ArgumentNullException(nameof(value));
            }

            if (!IsValid(value))
            {
                throw new ArgumentException(
                    $"External id whose length is more than {RoomConstants.GameHostExternalIdLength} is not available.",
                    nameof(value));
            }

            this.value = value.ToArray();
        }

        public static MatchMakerGameHostExternalId Empty { get; } =
            new MatchMakerGameHostExternalId(Array.Empty<byte>());

        private readonly byte[] value;

        public static MatchMakerGameHostExternalId Parse(byte[] value)
        {
            return new MatchMakerGameHostExternalId(value);
        }

        public static MatchMakerGameHostExternalId From<T>(T externalId)
        {
            switch (externalId)
            {
                case string s:
                    return new MatchMakerGameHostExternalId(
                        Utility.ConvertStringToFixedLengthArray(s, RoomConstants.GameHostExternalIdLength));
                case ulong v:
                    return new MatchMakerGameHostExternalId(Serializer.Serialize(v));
                case uint v:
                    return new MatchMakerGameHostExternalId(Serializer.Serialize(v));
                case ushort v:
                    return new MatchMakerGameHostExternalId(Serializer.Serialize(v));
                default:
                    throw new ArgumentException("Unsupported type.", nameof(externalId));
            }
        }

        public static bool TryParse(byte[] value, out MatchMakerGameHostExternalId externalId)
        {
            if (!IsValid(value))
            {
                externalId = null;
                return false;
            }

            externalId = new MatchMakerGameHostExternalId(value);
            return true;
        }

        public byte[] ToArray()
        {
            return value.ToArray();
        }

        public bool Equals(MatchMakerGameHostExternalId other)
        {
            return other != null && value.SequenceEqual(other.value);
        }

        public override bool Equals(object obj)
        {
            return Equals(obj as MatchMakerGameHostExternalId);
        }

        public override int GetHashCode()
        {
            unchecked
            {
                var hash = 17;
                foreach (var item in value)
                {
                    hash = hash * 31 + item;
                }

                return hash;
            }
        }

        public override string ToString()
        {
            return string.Join("", value.Select(b => $"{b:X2}"));
        }

        internal static bool IsValid(byte[] value)
        {
            return value != null && value.Length <= RoomConstants.GameHostExternalIdLength;
        }
    }

    public sealed class MatchMakerRoomPassword : IEquatable<MatchMakerRoomPassword>
    {
        public MatchMakerRoomPassword(string value)
        {
            if (value == null)
            {
                throw new ArgumentNullException(nameof(value));
            }

            if (!IsValid(value))
            {
                throw new ArgumentException(
                    $"A string whose length is more than {RoomConstants.RoomPasswordLength} is not available.",
                    nameof(value));
            }

            Value = value;
        }

        public static MatchMakerRoomPassword Empty { get; } = new MatchMakerRoomPassword("");

        public string Value { get; }

        public static MatchMakerRoomPassword Parse(string value)
        {
            return new MatchMakerRoomPassword(value);
        }

        public static bool TryParse(string value, out MatchMakerRoomPassword password)
        {
            if (!IsValid(value))
            {
                password = null;
                return false;
            }

            password = new MatchMakerRoomPassword(value);
            return true;
        }

        public bool Equals(MatchMakerRoomPassword other)
        {
            return other != null && string.Equals(Value, other.Value, StringComparison.Ordinal);
        }

        public override bool Equals(object obj)
        {
            return Equals(obj as MatchMakerRoomPassword);
        }

        public override int GetHashCode()
        {
            return StringComparer.Ordinal.GetHashCode(Value);
        }

        public override string ToString()
        {
            return Value;
        }

        internal static bool IsValid(string value)
        {
            return value != null && Encoding.UTF8.GetByteCount(value) <= RoomConstants.RoomPasswordLength;
        }
    }

    public sealed class MatchMakerSearchName : IEquatable<MatchMakerSearchName>
    {
        public MatchMakerSearchName(string value)
        {
            if (value == null)
            {
                throw new ArgumentNullException(nameof(value));
            }

            if (!IsValid(value))
            {
                throw new ArgumentException(
                    $"null string or string whose length is more than {ClientConstants.PlayerNameLength} is not available.",
                    nameof(value));
            }

            Value = value;
        }

        public static MatchMakerSearchName Empty { get; } = new MatchMakerSearchName("");

        public string Value { get; }

        public static MatchMakerSearchName Parse(string value)
        {
            return new MatchMakerSearchName(value);
        }

        public static bool TryParse(string value, out MatchMakerSearchName searchName)
        {
            if (!IsValid(value))
            {
                searchName = null;
                return false;
            }

            searchName = new MatchMakerSearchName(value);
            return true;
        }

        public bool Equals(MatchMakerSearchName other)
        {
            return other != null && string.Equals(Value, other.Value, StringComparison.Ordinal);
        }

        public override bool Equals(object obj)
        {
            return Equals(obj as MatchMakerSearchName);
        }

        public override int GetHashCode()
        {
            return StringComparer.Ordinal.GetHashCode(Value);
        }

        public override string ToString()
        {
            return Value;
        }

        internal static bool IsValid(string value)
        {
            return value != null && Encoding.UTF8.GetByteCount(value) <= ClientConstants.PlayerNameLength;
        }
    }
}
