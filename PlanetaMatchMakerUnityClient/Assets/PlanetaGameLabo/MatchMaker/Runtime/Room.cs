namespace PlanetaGameLabo.MatchMaker
{
    public sealed class RoomGroupInfo
    {
        public RoomGroupInfo(ListRoomGroupResultItem roomGroupResult)
        {
            name = roomGroupResult.Name;
        }

        /// <summary>
        /// A name of room group.
        /// </summary>
        public string name { get; }
    }

    public sealed class RoomInfo
    {
        public RoomInfo(byte roomGroupIndex, ListRoomResultItem roomResult)
        {
            this.roomGroupIndex = roomGroupIndex;
            roomId = roomResult.RoomId;
            hostPlayerFullName = roomResult.HostPlayerFullName;
            settingFlags = roomResult.SettingFlags;
            maxPlayerCount = roomResult.MaxPlayerCount;
            currentPlayerCount = roomResult.CurrentPlayerCount;
            createDatetime = roomResult.CreateDatetime;
        }

        /// <summary>
        /// An index of room group where this room exists.
        /// </summary>
        public byte roomGroupIndex { get; }

        /// <summary>
        /// An id of this room.
        /// </summary>
        public uint roomId { get; }

        /// <summary>
        /// An full name of player who owns this room.
        /// </summary>
        public PlayerFullName hostPlayerFullName { get; }

        /// <summary>
        /// A settings of this room
        /// </summary>
        public RoomSettingFlag settingFlags { get; }

        /// <summary>
        /// A max player count of this room.
        /// </summary>
        public byte maxPlayerCount { get; }

        /// <summary>
        /// A current player count of this room.
        /// </summary>
        public byte currentPlayerCount { get; }

        /// <summary>
        /// A datetime this room created.
        /// </summary>
        public Datetime createDatetime { get; }
    }

    public interface IReadOnlyHostingRoomInfo
    {
        /// <summary>
        /// An index of room group hosting room exist.
        /// </summary>
        byte roomGroupIndex { get; }

        /// <summary>
        /// An id of hosting room.
        /// </summary>
        uint roomId { get; }

        /// <summary>
        /// A max player count of hosting room.
        /// </summary>
        byte maxPlayerCount { get; }

        /// <summary>
        /// A current player count of hosting room.
        /// </summary>
        byte currentPlayerCount { get; }

        /// <summary>
        /// True of hosting room is public.
        /// </summary>
        bool isPublic { get; }

        /// <summary>
        /// True of hosting room is open.
        /// </summary>
        bool isOpen { get; }

        /// <summary>
        /// A password of hosting room.
        /// </summary>
        string password { get; }
    }

    public sealed class HostingRoomInfo : IReadOnlyHostingRoomInfo
    {
        public HostingRoomInfo(CreateRoomResult createRoomResult, byte roomGroupIndex, string password)
        {
            currentPlayerCount = createRoomResult.CurrentPlayerCount;
            isOpen = (createRoomResult.SettingFlags | RoomSettingFlag.OpenRoom) == RoomSettingFlag.OpenRoom;
            maxPlayerCount = createRoomResult.MaxPlayerCount;
            this.password = password;
            this.roomGroupIndex = roomGroupIndex;
            roomId = createRoomResult.RoomId;
        }

        public byte roomGroupIndex { get; set; }
        public uint roomId { get; set; }
        public byte maxPlayerCount { get; set; }
        public byte currentPlayerCount { get; set; }
        public bool isPublic => string.IsNullOrEmpty(password);
        public bool isOpen { get; set; }
        public string password { get; set; }
    }
}