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
}