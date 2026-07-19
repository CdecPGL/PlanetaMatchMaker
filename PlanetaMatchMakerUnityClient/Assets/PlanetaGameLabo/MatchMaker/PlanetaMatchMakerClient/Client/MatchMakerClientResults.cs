using System;

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
            ConnectionEstablishMode = info.ConnectionEstablishMode;
        }

        public uint RoomId { get; }
        public PlayerFullName HostPlayerFullName { get; }
        public RoomSettingFlag SettingFlags { get; }
        public byte MaxPlayerCount { get; }
        public byte CurrentPlayerCount { get; }
        public Datetime CreateDatetime { get; }
        public GameHostConnectionEstablishMode ConnectionEstablishMode { get; }
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
            string p2pServicePeerId)
        {
            ConnectionEstablishMode = connectionEstablishMode;
            P2pServicePeerId = new P2pServicePeerId(p2pServicePeerId);
        }

        /// <summary>
        /// A signaling method to connect to the host.
        /// </summary>
        public GameHostConnectionEstablishMode ConnectionEstablishMode { get; }

        /// <summary>
        /// The peer ID used to connect to the game host through the P2P service.
        /// </summary>
        public P2pServicePeerId P2pServicePeerId { get; }

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
            return new { SignalingMethod = ConnectionEstablishMode, P2pServicePeerId }.GetHashCode();
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
                   P2pServicePeerId == other.P2pServicePeerId;
        }
    }
}
