using System;
using System.Linq;
using System.Net;
using CdecPGL.MinimalSerializer;

namespace PlanetaGameLabo.MatchMaker
{
    public readonly struct ListRoomResultItem
    {
        internal ListRoomResultItem(ListRoomReplyMessage.RoomInfo info)
        {
            RoomId = info.RoomId;
            HostPlayerFullName = info.HostPlayerFullName;
            SettingFlags = info.SettingFlags;
            MaxPlayerCount = info.MaxPlayerCount;
            CurrentPlayerCount = info.CurrentPlayerCount;
            CreateDatetime = info.CreateDatetime;
        }

        public uint RoomId { get; }
        public PlayerFullName HostPlayerFullName { get; }
        public RoomSettingFlag SettingFlags { get; }
        public byte MaxPlayerCount { get; }
        public byte CurrentPlayerCount { get; }
        public Datetime CreateDatetime { get; }
    }

    public readonly struct CreateRoomResult
    {
        internal CreateRoomResult(uint roomId, RoomSettingFlag settingFlags, byte maxPlayerCount,
            byte currentPlayerCount)
        {
            RoomId = roomId;
            SettingFlags = settingFlags;
            MaxPlayerCount = maxPlayerCount;
            CurrentPlayerCount = currentPlayerCount;
        }

        public uint RoomId { get; }
        public RoomSettingFlag SettingFlags { get; }
        public byte MaxPlayerCount { get; }
        public byte CurrentPlayerCount { get; }
    }

    public readonly struct CreateRoomWithCreatingPortMappingResult
    {
        public CreateRoomWithCreatingPortMappingResult(bool isDefaultPortUsed, ushort usedPrivatePortFromCandidates,
            ushort usedPublicPortFromCandidates, CreateRoomResult creteRoomResult)
        {
            IsDefaultPortUsed = isDefaultPortUsed;
            UsedPrivatePortFromCandidates = usedPrivatePortFromCandidates;
            UsedPublicPortFromCandidates = usedPublicPortFromCandidates;
            CreteRoomResult = creteRoomResult;
        }

        public bool IsDefaultPortUsed { get; }
        public ushort UsedPrivatePortFromCandidates { get; }
        public ushort UsedPublicPortFromCandidates { get; }
        public CreateRoomResult CreteRoomResult { get; }
    }

    public readonly struct JoinRoomWithExternalServiceResult : IEquatable<JoinRoomWithExternalServiceResult>
    {
        internal JoinRoomWithExternalServiceResult(GameHostConnectionEstablishMode connectionEstablishMode,
            byte[] externalId)
        {
            ConnectionEstablishMode = connectionEstablishMode;
            ExternalId = externalId;
        }

        /// <summary>
        /// A signaling method to connect to the host.
        /// </summary>
        public GameHostConnectionEstablishMode ConnectionEstablishMode { get; }

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
                var other = (JoinRoomWithExternalServiceResult)obj;
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
            return new { SignalingMethod = ConnectionEstablishMode, externalIdString = externalIdByteString }
                .GetHashCode();
        }

        public static bool operator ==(JoinRoomWithExternalServiceResult left, JoinRoomWithExternalServiceResult right)
        {
            return left.Equals(right);
        }

        public static bool operator !=(JoinRoomWithExternalServiceResult left, JoinRoomWithExternalServiceResult right)
        {
            return !(left == right);
        }

        public bool Equals(JoinRoomWithExternalServiceResult other)
        {
            return ConnectionEstablishMode == other.ConnectionEstablishMode &&
                   ExternalId.SequenceEqual(other.ExternalId);
        }
    }
}