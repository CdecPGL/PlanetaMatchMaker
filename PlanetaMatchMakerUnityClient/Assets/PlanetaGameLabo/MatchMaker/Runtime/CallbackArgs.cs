using System.Collections.Generic;
using System.Net;

namespace PlanetaGameLabo.MatchMaker
{
    public struct ConnectCallbackArgs
    {
        public ConnectCallbackArgs(PlayerFullName playerFullName)
        {
            this.playerFullName = playerFullName;
        }

        public readonly PlayerFullName playerFullName;
    }

    public struct RequestRoomListCallbackArgs
    {
        public RequestRoomListCallbackArgs(byte totalRoomCount, byte startIndex,
            IReadOnlyList<RoomInfo> roomInfoList)
        {
            this.totalRoomCount = totalRoomCount;
            this.startIndex = startIndex;
            this.roomInfoList = roomInfoList;
        }

        public readonly byte totalRoomCount;
        public readonly byte startIndex;
        public readonly IReadOnlyList<RoomInfo> roomInfoList;
    }

    public struct HostRoomCallbackArgs
    {
        public HostRoomCallbackArgs(HostingRoomInfo hostRoomInfo)
        {
            this.hostRoomInfo = hostRoomInfo;
        }

        public readonly HostingRoomInfo hostRoomInfo;
    }

    public struct HostRoomWithCreatingPortMappingCallbackArgs
    {
        public HostRoomWithCreatingPortMappingCallbackArgs(HostingRoomInfo hostRoomInfo, bool isDefaultPortUsed,
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

    public struct JoinRoomCallbackArgs
    {
        public JoinRoomCallbackArgs(IPEndPoint roomGameHostEndPoint)
        {
            this.roomGameHostEndPoint = roomGameHostEndPoint;
        }

        public readonly IPEndPoint roomGameHostEndPoint;
    }
}