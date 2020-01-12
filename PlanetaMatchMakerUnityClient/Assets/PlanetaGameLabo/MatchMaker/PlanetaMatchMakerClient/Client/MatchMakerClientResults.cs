namespace PlanetaGameLabo.MatchMaker
{
    public struct RoomGroupResult
    {
        internal RoomGroupResult(ListRoomGroupReplyMessage.RoomGroupInfo info)
        {
            Name = info.Name;
        }

        public readonly string Name;
    }

    public struct RoomResult
    {
        internal RoomResult(ListRoomReplyMessage.RoomInfo info)
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

    public struct CreateRoomWithCreatingPortMappingResult
    {
        public readonly bool IsDefaultPortUsed;
        public readonly ushort UsedPrivatePortFromCandidates;
        public readonly ushort UsedPublicPortFromCandidates;

        public CreateRoomWithCreatingPortMappingResult(bool isDefaultPortUsed, ushort usedPrivatePortFromCandidates,
            ushort usedPublicPortFromCandidates)
        {
            IsDefaultPortUsed = isDefaultPortUsed;
            UsedPrivatePortFromCandidates = usedPrivatePortFromCandidates;
            UsedPublicPortFromCandidates = usedPublicPortFromCandidates;
        }
    }
}