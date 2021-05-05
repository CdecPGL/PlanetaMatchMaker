namespace PlanetaGameLabo.MatchMaker
{
    public struct ListRoomResultItem
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

        public readonly uint RoomId;
        public readonly PlayerFullName HostPlayerFullName;
        public readonly RoomSettingFlag SettingFlags;
        public readonly byte MaxPlayerCount;
        public readonly byte CurrentPlayerCount;
        public readonly Datetime CreateDatetime;
    }

    public struct CreateRoomResult
    {
        internal CreateRoomResult(uint roomId, RoomSettingFlag settingFlags, byte maxPlayerCount,
            byte currentPlayerCount)
        {
            RoomId = roomId;
            SettingFlags = settingFlags;
            MaxPlayerCount = maxPlayerCount;
            CurrentPlayerCount = currentPlayerCount;
        }

        public readonly uint RoomId;
        public readonly RoomSettingFlag SettingFlags;
        public readonly byte MaxPlayerCount;
        public readonly byte CurrentPlayerCount;
    }

    public struct CreateRoomWithCreatingPortMappingResult
    {
        public CreateRoomWithCreatingPortMappingResult(bool isDefaultPortUsed, ushort usedPrivatePortFromCandidates,
            ushort usedPublicPortFromCandidates, CreateRoomResult creteRoomResult)
        {
            IsDefaultPortUsed = isDefaultPortUsed;
            UsedPrivatePortFromCandidates = usedPrivatePortFromCandidates;
            UsedPublicPortFromCandidates = usedPublicPortFromCandidates;
            CreteRoomResult = creteRoomResult;
        }

        public readonly bool IsDefaultPortUsed;
        public readonly ushort UsedPrivatePortFromCandidates;
        public readonly ushort UsedPublicPortFromCandidates;
        public readonly CreateRoomResult CreteRoomResult;
    }
}