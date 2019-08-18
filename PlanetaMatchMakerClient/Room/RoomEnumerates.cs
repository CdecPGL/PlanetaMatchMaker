using System;

namespace PlanetaGameLabo.MatchMaker
{
    public enum RoomStatus : byte
    {
        Open,
        Close,
        Remove
    }

    [Flags]
    public enum RoomSettingFlag : byte
    {
        None = 0,
        PublicRoom = 1,
        OpenRoom = 2
    };

    [Flags]
    public enum RoomSearchTargetFlag : byte
    {
        None = 0,
        PublicRoom = 1,
        PrivateRoom = 2,
        OpenRoom = 4,
        ClosedRoom = 8
    }

    public enum RoomDataSortKind : byte
    {
        NameAscending,
        NameDescending,
        CreateDatetimeAscending,
        CreateDatetimeDescending
    }
}