using System;
using System.Globalization;
using System.Linq;
using System.Net;
using System.Text;
using System.Text.RegularExpressions;
using CdecPGL.MinimalSerializer;

namespace PlanetaGameLabo.MatchMaker
{
    public sealed class GameId : IEquatable<GameId>
    {
        public GameId(string value)
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

        public static GameId Parse(string value)
        {
            return new GameId(value);
        }

        public static bool TryParse(string value, out GameId gameId)
        {
            if (!IsValid(value))
            {
                gameId = null;
                return false;
            }

            gameId = new GameId(value);
            return true;
        }

        public bool Equals(GameId other)
        {
            return other != null && string.Equals(Value, other.Value, StringComparison.Ordinal);
        }

        public override bool Equals(object obj)
        {
            return Equals(obj as GameId);
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

    public sealed class GameVersion : IEquatable<GameVersion>
    {
        public GameVersion(string value)
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

        public static GameVersion Empty { get; } = new GameVersion("");

        public string Value { get; }

        public static GameVersion Parse(string value)
        {
            return new GameVersion(value);
        }

        public static bool TryParse(string value, out GameVersion gameVersion)
        {
            if (!IsValid(value))
            {
                gameVersion = null;
                return false;
            }

            gameVersion = new GameVersion(value);
            return true;
        }

        public bool Equals(GameVersion other)
        {
            return other != null && string.Equals(Value, other.Value, StringComparison.Ordinal);
        }

        public override bool Equals(object obj)
        {
            return Equals(obj as GameVersion);
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
    /// Immutable host or IP address for PMMS server connections.
    /// IPv4, IPv6 or host name is valid.
    /// </summary>
    public sealed class Host : IEquatable<Host>
    {
        public Host(string value)
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

        public static Host Parse(string value)
        {
            return new Host(value);
        }

        public static bool TryParse(string value, out Host host)
        {
            if (!TryNormalize(value, out var normalizedValue))
            {
                host = null;
                return false;
            }

            host = new Host(normalizedValue);
            return true;
        }

        public bool Equals(Host other)
        {
            return other != null && string.Equals(Value, other.Value, StringComparison.OrdinalIgnoreCase);
        }

        public override bool Equals(object obj)
        {
            return Equals(obj as Host);
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

    public sealed class PlayerName : IEquatable<PlayerName>
    {
        public PlayerName(string value)
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

        public static PlayerName Parse(string value)
        {
            return new PlayerName(value);
        }

        public static bool TryParse(string value, out PlayerName playerName)
        {
            if (!IsValid(value))
            {
                playerName = null;
                return false;
            }

            playerName = new PlayerName(value);
            return true;
        }

        public bool Equals(PlayerName other)
        {
            return other != null && string.Equals(Value, other.Value, StringComparison.Ordinal);
        }

        public override bool Equals(object obj)
        {
            return Equals(obj as PlayerName);
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

    public sealed class GameHostPort : IEquatable<GameHostPort>
    {
        public GameHostPort(ushort value)
        {
            if (!IsValid(value))
            {
                throw new ArgumentException("Dynamic/private port is available.", nameof(value));
            }

            Value = value;
        }

        public ushort Value { get; }

        public static GameHostPort Parse(ushort value)
        {
            return new GameHostPort(value);
        }

        public static bool TryParse(ushort value, out GameHostPort port)
        {
            if (!IsValid(value))
            {
                port = null;
                return false;
            }

            port = new GameHostPort(value);
            return true;
        }

        public bool Equals(GameHostPort other)
        {
            return other != null && Value == other.Value;
        }

        public override bool Equals(object obj)
        {
            return Equals(obj as GameHostPort);
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

    public sealed class GameHostExternalId : IEquatable<GameHostExternalId>
    {
        public GameHostExternalId(byte[] value)
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

        public static GameHostExternalId Empty { get; } =
            new GameHostExternalId(Array.Empty<byte>());

        private readonly byte[] value;

        public static GameHostExternalId Parse(byte[] value)
        {
            return new GameHostExternalId(value);
        }

        public static GameHostExternalId FromString(string value)
        {
            if (value == null)
            {
                throw new ArgumentNullException(nameof(value));
            }

            return new GameHostExternalId(
                Utility.ConvertStringToFixedLengthArray(value, RoomConstants.GameHostExternalIdLength));
        }

        public static GameHostExternalId FromUInt64(ulong value)
        {
            return new GameHostExternalId(Serializer.Serialize(value));
        }

        public static GameHostExternalId FromUInt32(uint value)
        {
            return new GameHostExternalId(Serializer.Serialize(value));
        }

        public static GameHostExternalId FromUInt16(ushort value)
        {
            return new GameHostExternalId(Serializer.Serialize(value));
        }

        public static bool TryParse(byte[] value, out GameHostExternalId externalId)
        {
            if (!IsValid(value))
            {
                externalId = null;
                return false;
            }

            externalId = new GameHostExternalId(value);
            return true;
        }

        public byte[] ToArray()
        {
            return value.ToArray();
        }

        public bool Equals(GameHostExternalId other)
        {
            return other != null && value.SequenceEqual(other.value);
        }

        public override bool Equals(object obj)
        {
            return Equals(obj as GameHostExternalId);
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

    public sealed class RoomPassword : IEquatable<RoomPassword>
    {
        public RoomPassword(string value)
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

        public static RoomPassword Empty { get; } = new RoomPassword("");

        public string Value { get; }

        public static RoomPassword Parse(string value)
        {
            return new RoomPassword(value);
        }

        public static bool TryParse(string value, out RoomPassword password)
        {
            if (!IsValid(value))
            {
                password = null;
                return false;
            }

            password = new RoomPassword(value);
            return true;
        }

        public bool Equals(RoomPassword other)
        {
            return other != null && string.Equals(Value, other.Value, StringComparison.Ordinal);
        }

        public override bool Equals(object obj)
        {
            return Equals(obj as RoomPassword);
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

    public sealed class SearchName : IEquatable<SearchName>
    {
        public SearchName(string value)
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

        public static SearchName Empty { get; } = new SearchName("");

        public string Value { get; }

        public static SearchName Parse(string value)
        {
            return new SearchName(value);
        }

        public static bool TryParse(string value, out SearchName searchName)
        {
            if (!IsValid(value))
            {
                searchName = null;
                return false;
            }

            searchName = new SearchName(value);
            return true;
        }

        public bool Equals(SearchName other)
        {
            return other != null && string.Equals(Value, other.Value, StringComparison.Ordinal);
        }

        public override bool Equals(object obj)
        {
            return Equals(obj as SearchName);
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
