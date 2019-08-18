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
            Name = info.Name;
            SettingFlags = info.SettingFlags;
            MaxPlayerCount = info.MaxPlayerCount;
            CurrentPlayerCount = info.CurrentPlayerCount;
            CreateDatetime = info.CreateDatetime;
        }

        public readonly uint RoomId;
        public readonly string Name;
        public readonly RoomSettingFlag SettingFlags;
        public readonly byte MaxPlayerCount;
        public readonly byte CurrentPlayerCount;
        public readonly Datetime CreateDatetime;
    }
}
