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
            var commandMap = new Dictionary<Command, ICommandExecutor>
            {
                {Command.StressTestConnectAndStay, new StressTestConnectAndStayCommandExecutor(outputStream)},
                {
                    Command.StressTestConnectAndDisconnect,
                    new StressTestConnectAndDisconnectCommandExecutor(outputStream)
                },
                {Command.StressTestGetRoomGroupList, new StressTestGetRoomGroupListCommandExecutor(outputStream)}
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
        UpdateRoomStatus,
        ConnectionTest,
        CreatePortMapping,
        JoinRoomWithCreatingPortMapping,
        StressTestConnectAndStay,
        StressTestConnectAndDisconnect,
        StressTestGetRoomGroupList
    };
}