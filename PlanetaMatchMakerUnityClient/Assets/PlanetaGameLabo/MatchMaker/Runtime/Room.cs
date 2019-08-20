using System;

namespace PlanetaGameLabo.MatchMaker
{
    public interface IRoomInfo
    {
        byte roomGroupIndex { get; }
        uint roomId { get; }
        string name { get; }
        RoomSettingFlag settingFlags { get; }
        byte maxPlayerCount { get; }
        byte currentPlayerCount { get; }
        Datetime createDatetime { get; }
    }

    public interface IHostingRoomInfo
    {
        byte roomGroupIndex { get; }
        uint roomId { get; }
        string name { get; }
        byte maxPlayerCount { get; }
    }

    public interface IRoomGroupInfo
    {
        string name { get; }
    }
}