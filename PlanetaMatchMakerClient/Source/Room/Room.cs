using System;
using System.Linq;
using System.Net;
using CdecPGL.MinimalSerializer;

namespace PlanetaGameLabo.MatchMaker
{
    public readonly struct RoomHostEndpoint : IEquatable<RoomHostEndpoint>
    {
        internal RoomHostEndpoint(GameHostConnectionEstablishMode connectionEstablishMode, IPEndPoint ipEndpoint,
            byte[] externalId)
        {
            ConnectionEstablishMode = connectionEstablishMode;
            IPEndpoint = ipEndpoint;
            ExternalId = externalId;
        }

        /// <summary>
        /// A signaling method to connect to the host.
        /// </summary>
        public GameHostConnectionEstablishMode ConnectionEstablishMode { get; }

        /// <summary>
        /// An IP endpoint of the game host when signaling method is direct.
        /// </summary>
        public IPEndPoint IPEndpoint { get; }

        /// <summary>
        /// An external service ID of game host when signaling method is external service.
        /// </summary>
        public byte[] ExternalId { get; }

        /// <summary>
        /// Get external id as UTF-8 string.
        /// </summary>
        /// <exception cref="ArgumentException">The external id is not UTF-8 string.</exception>
        /// <returns></returns>
        public string GetExternalIdAsString()
        {
            return Utility.ConvertFixedLengthArrayToString(ExternalId);
        }

        /// <summary>
        /// Get external id as uint64.
        /// </summary>
        /// <returns></returns>
        public ulong GetExternalIdAsUInt64()
        {
            var size = Serializer.GetSerializedSize<ulong>();
            var data = new byte[size];
            Array.Copy(ExternalId, data, size);
            return Serializer.Deserialize<ulong>(data);
        }

        /// <summary>
        /// Get external id as uint32.
        /// </summary>
        /// <returns></returns>
        public uint GetExternalIdAsUInt32()
        {
            var size = Serializer.GetSerializedSize<uint>();
            var data = new byte[size];
            Array.Copy(ExternalId, data, size);
            return Serializer.Deserialize<uint>(data);
        }

        /// <summary>
        /// Get external id as uint16.
        /// </summary>
        /// <returns></returns>
        public ushort GetExternalIdAsUInt16()
        {
            var size = Serializer.GetSerializedSize<ushort>();
            var data = new byte[size];
            Array.Copy(ExternalId, data, size);
            return Serializer.Deserialize<ushort>(data);
        }

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
            return new
            {
                SignalingMethod = ConnectionEstablishMode, IPEndpoint, externalIdString = externalIdByteString
            }.GetHashCode();
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
            return ConnectionEstablishMode == other.ConnectionEstablishMode && IPEndpoint.Equals(other.IPEndpoint) &&
                   ExternalId.SequenceEqual(other.ExternalId);
        }
    }
}