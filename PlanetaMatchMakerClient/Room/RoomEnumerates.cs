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
    public enum RoomFlag : byte
    {
        IsPrivate = 1,
        IsOpen = 2
    }

    public enum RoomDataSortKind : byte
    {
        NameAscending,
        NameDescending,
        CreateDatetimeAscending,
        CreateDatetimeDescending
    }
}
