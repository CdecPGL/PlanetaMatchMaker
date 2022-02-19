using System.Collections.Generic;
using System.Net;

namespace PlanetaGameLabo.MatchMaker
{
    public readonly struct ConnectResult
    {
        public ConnectResult(PlayerFullName playerFullName)
        {
            this.playerFullName = playerFullName;
        }

        public readonly PlayerFullName playerFullName;
    }

    public readonly struct RequestRoomListResult
    {
        public RequestRoomListResult(ushort totalRoomCount, ushort startIndex,
            IReadOnlyList<RoomInfo> roomInfoList)
        {
            this.totalRoomCount = totalRoomCount;
            this.startIndex = startIndex;
            this.roomInfoList = roomInfoList;
        }

        public readonly ushort totalRoomCount;
        public readonly ushort startIndex;
        public readonly IReadOnlyList<RoomInfo> roomInfoList;
    }

    public readonly struct HostRoomResult
    {
        public HostRoomResult(IReadOnlyHostingRoomInfo hostRoomInfo)
        {
            this.hostRoomInfo = hostRoomInfo;
        }

        public readonly IReadOnlyHostingRoomInfo hostRoomInfo;
    }

    public readonly struct HostRoomWithCreatingPortMappingResult
    {
        public HostRoomWithCreatingPortMappingResult(HostingRoomInfo hostRoomInfo, bool isDefaultPortUsed,
            ushort privatePort, ushort publicPort)
        {
            this.hostRoomInfo = hostRoomInfo;
            this.isDefaultPortUsed = isDefaultPortUsed;
            this.privatePort = privatePort;
            this.publicPort = publicPort;
        }

        public readonly HostingRoomInfo hostRoomInfo;
        public readonly bool isDefaultPortUsed;
        public readonly ushort privatePort;
        public readonly ushort publicPort;
    }

    public readonly struct JoinRoomResult
    {
        public JoinRoomResult(IPEndPoint roomGameHostEndPoint)
        {
            this.roomGameHostEndPoint = roomGameHostEndPoint;
        }

        public readonly IPEndPoint roomGameHostEndPoint;
    }
}