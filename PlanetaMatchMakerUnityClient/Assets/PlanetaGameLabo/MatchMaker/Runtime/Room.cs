namespace PlanetaGameLabo.MatchMaker
{
    public sealed class RoomGroupInfo
    {
        public RoomGroupInfo(RoomGroupResult roomGroupResult)
        {
            name = roomGroupResult.Name;
        }

        public string name { get; }
    }

    public sealed class RoomInfo
    {
        public RoomInfo(byte roomGroupIndex, RoomResult roomResult)
        {
            this.roomGroupIndex = roomGroupIndex;
            roomId = roomResult.RoomId;
            hostPlayerFullName = roomResult.HostPlayerFullName;
            settingFlags = roomResult.SettingFlags;
            maxPlayerCount = roomResult.MaxPlayerCount;
            currentPlayerCount = roomResult.CurrentPlayerCount;
            createDatetime = roomResult.CreateDatetime;
        }

        public byte roomGroupIndex { get; }
        public uint roomId { get; }
        public PlayerFullName hostPlayerFullName { get; }
        public RoomSettingFlag settingFlags { get; }
        public byte maxPlayerCount { get; }
        public byte currentPlayerCount { get; }
        public Datetime createDatetime { get; }
    }

    public sealed class HostingRoomInfo
    {
        public HostingRoomInfo(byte roomGroupIndex, uint roomId, byte maxPlayerCount, string password)
        {
            this.roomGroupIndex = roomGroupIndex;
            this.roomId = roomId;
            this.maxPlayerCount = maxPlayerCount;
            this.password = password;
        }

        public byte roomGroupIndex { get; }
        public uint roomId { get; }
        public byte maxPlayerCount { get; }
        public bool isPublic => string.IsNullOrEmpty(password);
        public string password { get; }
    }
}