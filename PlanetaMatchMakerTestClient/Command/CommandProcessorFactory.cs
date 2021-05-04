using System;
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
                new ListRoomCommandExecutor(outputStream), new JoinRoomCommandExecutor(outputStream),
                new UpdateHostingRoomStatusCommandExecutor(outputStream),
                new ConnectionTestCommandExecutor(outputStream), new CreatePortMappingCommandExecutor(outputStream),
                new ShowPortMappingsCommandExecutor(outputStream),
                new ReleasePortMappingsCommandExecutor(outputStream),
                new CreateRoomWithCreatingPortMappingCommandExecutor(outputStream),
                new TestAllCommandExecutor(outputStream), new StressTestConnectAndStayCommandExecutor(outputStream),
                new StressTestConnectAndDisconnectCommandExecutor(outputStream),
                new StressTestGetRoomListCommandExecutor(outputStream)
            };
            return new CommandProcessor(outputStream, commandMap);
        }
    }

    internal enum Command
    {
        Connect,
        Disconnect,
        CreateRoom,
        ListRoom,
        JoinRoom,
        UpdateHostingRoomStatus,
        ConnectionTest,
        CreatePortMapping,
        ShowPortMappings,
        ReleasePortMappings,
        CreateRoomWithCreatingPortMapping,
        TestAll,
        StressTestConnectAndStay,
        StressTestConnectAndDisconnect,
        StressTestGetRoomList
    };
}