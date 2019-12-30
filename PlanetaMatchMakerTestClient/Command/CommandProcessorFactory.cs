using System;
using System.Collections.Generic;
using System.IO;

namespace PlanetaGameLabo.MatchMaker
{
    internal static class CommandProcessorFactory
    {
        public static CommandProcessor Create()
        {
            var outputStream =
                new StreamWriter(System.Console.OpenStandardOutput(), Console.OutputEncoding) {AutoFlush = true};
            var commandMap = new ICommandExecutor[]
            {
                new ConnectCommandExecutor(outputStream), new DisconnectCommandExecutor(outputStream),
                new ListRoomGroupCommandExecutor(outputStream), new CreateRoomCommandExecutor(outputStream),
                new ListRoomCommandExecutor(outputStream), new JoinRoomCommandExecutor(outputStream),
                new UpdateHostingRoomStatusCommandExecutor(outputStream),
                new StressTestConnectAndStayCommandExecutor(outputStream),
                new StressTestConnectAndDisconnectCommandExecutor(outputStream),
                new StressTestGetRoomGroupListCommandExecutor(outputStream)
            };
            return new CommandProcessor(outputStream, commandMap);
        }
    }

    internal enum Command
    {
        Connect,
        Disconnect,
        ListRoomGroup,
        CreateRoom,
        ListRoom,
        JoinRoom,
        UpdateHostingRoomStatus,
        ConnectionTest,
        CreatePortMapping,
        JoinRoomWithCreatingPortMapping,
        StressTestConnectAndStay,
        StressTestConnectAndDisconnect,
        StressTestGetRoomGroupList
    };
}