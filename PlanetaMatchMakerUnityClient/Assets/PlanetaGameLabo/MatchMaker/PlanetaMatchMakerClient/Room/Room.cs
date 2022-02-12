using System;
using System.Linq;
using System.Net;

namespace PlanetaGameLabo.MatchMaker
{
    public readonly struct RoomHostEndpoint : IEquatable<RoomHostEndpoint>
    {
        internal RoomHostEndpoint(GameHostSignalingMethod signalingMethod, IPEndPoint ipEndpoint, byte[] externalId)
        {
            SignalingMethod = signalingMethod;
            IPEndpoint = ipEndpoint;
            ExternalId = externalId;

            try
            {
                ExternalIdString = Utility.ConvertFixedLengthArrayToString(ExternalId);
            }
            catch (ArgumentException)
            {
                ExternalIdString = null;
            }
        }

        /// <summary>
        /// A signaling method to connect to the host.
        /// </summary>
        public GameHostSignalingMethod SignalingMethod { get; }

        /// <summary>
        /// An IP endpoint of the game host when signaling method is direct.
        /// </summary>
        public IPEndPoint IPEndpoint { get; }

        /// <summary>
        /// An external service ID of game host when signaling method is external service.
        /// </summary>
        public byte[] ExternalId { get; }

        /// <summary>
        /// An string which is generated from the external service ID as UTF-8 of game host when signaling method is external service.
        /// This property will be null if external service ID is not UTF-8 string.
        /// </summary>
        public string ExternalIdString { get; }

        public override bool Equals(object obj)
        {
            if (obj == null)
            {
                return false;
            }

            try
            {
                var other = (RoomHostEndpoint)obj;
                return Equals(other);
            }
            catch (InvalidCastException)
            {
                return false;
            }
        }

        public override int GetHashCode()
        {
            var externalIdByteString = string.Join("", ExternalId.Select(b => $"{b:X00}"));
            return new { SignalingMethod, IPEndpoint, externalIdString = externalIdByteString }.GetHashCode();
        }

        public static bool operator ==(RoomHostEndpoint left, RoomHostEndpoint right)
        {
            return left.Equals(right);
        }

        public static bool operator !=(RoomHostEndpoint left, RoomHostEndpoint right)
        {
            return !(left == right);
        }

        public bool Equals(RoomHostEndpoint other)
        {
            return SignalingMethod == other.SignalingMethod && IPEndpoint.Equals(other.IPEndpoint) &&
                   ExternalId.SequenceEqual(other.ExternalId);
        }
    }
}