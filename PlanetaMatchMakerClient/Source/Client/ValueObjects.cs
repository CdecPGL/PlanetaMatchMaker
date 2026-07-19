using System;
using System.Globalization;
using System.Linq;
using System.Net;
using System.Text;
using System.Text.RegularExpressions;
using CdecPGL.MinimalSerializer;

namespace PlanetaGameLabo.MatchMaker
{
    public readonly struct GameId : IEquatable<GameId>
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

            this.value = value;
        }

        private readonly string value;

        public string Value
        {
            get
            {
                if (value == null)
                {
                    throw new InvalidOperationException("Default GameId is not valid.");
                }

                return value;
            }
        }

        public static GameId Parse(string value)
        {
            return new GameId(value);
        }

        public static bool TryParse(string value, out GameId gameId)
        {
            if (!IsValid(value))
            {
                gameId = default;
                return false;
            }

            gameId = new GameId(value);
            return true;
        }

        public bool Equals(GameId other)
        {
            return string.Equals(value, other.value, StringComparison.Ordinal);
        }

        public static bool operator ==(GameId left, GameId right)
        {
            return left.Equals(right);
        }

        public static bool operator !=(GameId left, GameId right)
        {
            return !left.Equals(right);
        }

        public override bool Equals(object obj)
        {
            return obj is GameId other && Equals(other);
        }

        public override int GetHashCode()
        {
            return value == null ? 0 : StringComparer.Ordinal.GetHashCode(value);
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

    public readonly struct GameVersion : IEquatable<GameVersion>
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

            this.value = value;
        }

        public static GameVersion Empty { get; } = new GameVersion("");

        private readonly string value;

        public string Value => value ?? "";

        public static GameVersion Parse(string value)
        {
            return new GameVersion(value);
        }

        public static bool TryParse(string value, out GameVersion gameVersion)
        {
            if (!IsValid(value))
            {
                gameVersion = default;
                return false;
            }

            gameVersion = new GameVersion(value);
            return true;
        }

        public bool Equals(GameVersion other)
        {
            return string.Equals(Value, other.Value, StringComparison.Ordinal);
        }

        public static bool operator ==(GameVersion left, GameVersion right)
        {
            return left.Equals(right);
        }

        public static bool operator !=(GameVersion left, GameVersion right)
        {
            return !left.Equals(right);
        }

        public override bool Equals(object obj)
        {
            return obj is GameVersion other && Equals(other);
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
    public readonly struct Host : IEquatable<Host>
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

            this.value = normalizedValue;
        }

        private readonly string value;

        public string Value
        {
            get
            {
                if (value == null)
                {
                    throw new InvalidOperationException("Default Host is not valid.");
                }

                return value;
            }
        }

        public static Host Parse(string value)
        {
            return new Host(value);
        }

        public static bool TryParse(string value, out Host host)
        {
            if (!TryNormalize(value, out var normalizedValue))
            {
                host = default;
                return false;
            }

            host = new Host(normalizedValue);
            return true;
        }

        public bool Equals(Host other)
        {
            return string.Equals(value, other.value, StringComparison.OrdinalIgnoreCase);
        }

        public static bool operator ==(Host left, Host right)
        {
            return left.Equals(right);
        }

        public static bool operator !=(Host left, Host right)
        {
            return !left.Equals(right);
        }

        public override bool Equals(object obj)
        {
            return obj is Host other && Equals(other);
        }

        public override int GetHashCode()
        {
            return value == null ? 0 : StringComparer.OrdinalIgnoreCase.GetHashCode(value);
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

    public readonly struct ServerPort : IEquatable<ServerPort>
    {
        public ServerPort(ushort value)
        {
            if (!IsValid(value))
            {
                throw new ArgumentException("0 is not available.", nameof(value));
            }

            this.value = value;
        }

        private readonly ushort value;

        public ushort Value
        {
            get
            {
                if (!IsValid(value))
                {
                    throw new InvalidOperationException("Default ServerPort is not valid.");
                }

                return value;
            }
        }

        public static ServerPort Parse(ushort value)
        {
            return new ServerPort(value);
        }

        public static bool TryParse(ushort value, out ServerPort port)
        {
            if (!IsValid(value))
            {
                port = default;
                return false;
            }

            port = new ServerPort(value);
            return true;
        }

        public bool Equals(ServerPort other)
        {
            return value == other.value;
        }

        public static bool operator ==(ServerPort left, ServerPort right)
        {
            return left.Equals(right);
        }

        public static bool operator !=(ServerPort left, ServerPort right)
        {
            return !left.Equals(right);
        }

        public override bool Equals(object obj)
        {
            return obj is ServerPort other && Equals(other);
        }

        public override int GetHashCode()
        {
            return value.GetHashCode();
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

    public readonly struct PlayerName : IEquatable<PlayerName>
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

            this.value = value;
        }

        private readonly string value;

        public string Value
        {
            get
            {
                if (value == null)
                {
                    throw new InvalidOperationException("Default PlayerName is not valid.");
                }

                return value;
            }
        }

        public static PlayerName Parse(string value)
        {
            return new PlayerName(value);
        }

        public static bool TryParse(string value, out PlayerName playerName)
        {
            if (!IsValid(value))
            {
                playerName = default;
                return false;
            }

            playerName = new PlayerName(value);
            return true;
        }

        public bool Equals(PlayerName other)
        {
            return string.Equals(value, other.value, StringComparison.Ordinal);
        }

        public static bool operator ==(PlayerName left, PlayerName right)
        {
            return left.Equals(right);
        }

        public static bool operator !=(PlayerName left, PlayerName right)
        {
            return !left.Equals(right);
        }

        public override bool Equals(object obj)
        {
            return obj is PlayerName other && Equals(other);
        }

        public override int GetHashCode()
        {
            return value == null ? 0 : StringComparer.Ordinal.GetHashCode(value);
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

    public readonly struct GameHostPort : IEquatable<GameHostPort>
    {
        public GameHostPort(ushort value)
        {
            if (!IsValid(value))
            {
                throw new ArgumentException("Dynamic/private port is available.", nameof(value));
            }

            this.value = value;
        }

        private readonly ushort value;

        public ushort Value
        {
            get
            {
                if (!IsValid(value))
                {
                    throw new InvalidOperationException("Default GameHostPort is not valid.");
                }

                return value;
            }
        }

        public static GameHostPort Parse(ushort value)
        {
            return new GameHostPort(value);
        }

        public static bool TryParse(ushort value, out GameHostPort port)
        {
            if (!IsValid(value))
            {
                port = default;
                return false;
            }

            port = new GameHostPort(value);
            return true;
        }

        public bool Equals(GameHostPort other)
        {
            return value == other.value;
        }

        public static bool operator ==(GameHostPort left, GameHostPort right)
        {
            return left.Equals(right);
        }

        public static bool operator !=(GameHostPort left, GameHostPort right)
        {
            return !left.Equals(right);
        }

        public override bool Equals(object obj)
        {
            return obj is GameHostPort other && Equals(other);
        }

        public override int GetHashCode()
        {
            return value.GetHashCode();
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

    public readonly struct P2pServicePeerId : IEquatable<P2pServicePeerId>
    {
        public P2pServicePeerId(string value)
        {
            if (value == null)
            {
                throw new ArgumentNullException(nameof(value));
            }

            if (!IsValid(value))
            {
                throw new ArgumentException(
                    $"P2P service peer ID must be valid UTF-8 without embedded NUL and at most {RoomConstants.P2pServicePeerIdLength} bytes.",
                    nameof(value));
            }

            this.value = value;
        }

        public static P2pServicePeerId Empty { get; } = new P2pServicePeerId("");

        private readonly string value;

        public string Value => value ?? "";

        public static P2pServicePeerId Parse(string value)
        {
            return new P2pServicePeerId(value);
        }

        public static bool TryParse(string value, out P2pServicePeerId peerId)
        {
            if (!IsValid(value))
            {
                peerId = default;
                return false;
            }

            peerId = new P2pServicePeerId(value);
            return true;
        }

        public bool Equals(P2pServicePeerId other)
        {
            return string.Equals(Value, other.Value, StringComparison.Ordinal);
        }

        public static bool operator ==(P2pServicePeerId left, P2pServicePeerId right)
        {
            return left.Equals(right);
        }

        public static bool operator !=(P2pServicePeerId left, P2pServicePeerId right)
        {
            return !left.Equals(right);
        }

        public override bool Equals(object obj)
        {
            return obj is P2pServicePeerId other && Equals(other);
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
            if (value == null || value.IndexOf('\0') >= 0)
            {
                return false;
            }

            try
            {
                return new UTF8Encoding(false, true).GetByteCount(value) <= RoomConstants.P2pServicePeerIdLength;
            }
            catch (EncoderFallbackException)
            {
                return false;
            }
        }
    }

    public readonly struct RoomPassword : IEquatable<RoomPassword>
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

            this.value = value;
        }

        public static RoomPassword Empty { get; } = new RoomPassword("");

        private readonly string value;

        public string Value => value ?? "";

        public static RoomPassword Parse(string value)
        {
            return new RoomPassword(value);
        }

        public static bool TryParse(string value, out RoomPassword password)
        {
            if (!IsValid(value))
            {
                password = default;
                return false;
            }

            password = new RoomPassword(value);
            return true;
        }

        public bool Equals(RoomPassword other)
        {
            return string.Equals(Value, other.Value, StringComparison.Ordinal);
        }

        public static bool operator ==(RoomPassword left, RoomPassword right)
        {
            return left.Equals(right);
        }

        public static bool operator !=(RoomPassword left, RoomPassword right)
        {
            return !left.Equals(right);
        }

        public override bool Equals(object obj)
        {
            return obj is RoomPassword other && Equals(other);
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

    public readonly struct SearchName : IEquatable<SearchName>
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

            this.value = value;
        }

        public static SearchName Empty { get; } = new SearchName("");

        private readonly string value;

        public string Value => value ?? "";

        public static SearchName Parse(string value)
        {
            return new SearchName(value);
        }

        public static bool TryParse(string value, out SearchName searchName)
        {
            if (!IsValid(value))
            {
                searchName = default;
                return false;
            }

            searchName = new SearchName(value);
            return true;
        }

        public bool Equals(SearchName other)
        {
            return string.Equals(Value, other.Value, StringComparison.Ordinal);
        }

        public static bool operator ==(SearchName left, SearchName right)
        {
            return left.Equals(right);
        }

        public static bool operator !=(SearchName left, SearchName right)
        {
            return !left.Equals(right);
        }

        public override bool Equals(object obj)
        {
            return obj is SearchName other && Equals(other);
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
