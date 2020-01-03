using System.Collections.Generic;
using System.IO;
using System.Threading;
using System.Threading.Tasks;

namespace PlanetaGameLabo.MatchMaker
{
    internal class
        CreateRoomWithCreatingPortMappingCommandExecutor : StandardCommandExecutorBase<
            CreateRoomWithCreatingPortMappingCommandOptions>
    {
        public override string Explanation => "Create room to indicated room group with creating port mapping if need.";
        public override Command command => Command.CreateRoomWithCreatingPortMapping;

        public CreateRoomWithCreatingPortMappingCommandExecutor(StreamWriter outputStream) : base(outputStream)
        {
        }

        protected override async Task ExecuteStandardCommand(MatchMakerClient sharedClient,
            CreateRoomWithCreatingPortMappingCommandOptions options,
            CancellationToken cancellationToken)
        {
            await sharedClient.CreateRoomWithCreatingPortMappingAsync(options.RoomGroupIndex, options.Name,
                options.MaxPlayerCount, options.DiscoverTimeoutMilliSeconds, options.Protocol, options.PortCandidates,
                options.DefaultPortNumber, options.IsPublic, options.Password);
            OutputStream.WriteLine(
                $"Room created with id \"{sharedClient.HostingRoomId}\" in room group {sharedClient.HostingRoomGroupIndex}.");
        }
    }

    internal class CreateRoomWithCreatingPortMappingCommandOptions : StandardCommandOptions
    {
        [CommandLine.Value(0, MetaName = "RoomGroupIndex", Required = true,
            HelpText = "An index of room group where room is created.")]
        public byte RoomGroupIndex { get; set; }

        [CommandLine.Value(1, MetaName = "RoomName", Required = true,
            HelpText = "A name of room to create.")]
        public string Name { get; set; }

        [CommandLine.Value(2, MetaName = "MaxPlayerCount", Required = true,
            HelpText = "Max player count of room.")]
        public byte MaxPlayerCount { get; set; }

        [CommandLine.Value(3, MetaName = "Protocol", Required = true,
            HelpText = "Transport protocol used to host game.")]
        public TransportProtocol Protocol { get; set; }

        [CommandLine.Value(4, MetaName = "DefaultPortNumber", Required = true,
            HelpText = "A default port number which is used in hosting game.")]
        public ushort DefaultPortNumber { get; set; }

        [CommandLine.Value(5, MetaName = "DiscoverTimeoutMilliSeconds", Required = true,
            HelpText = "Timeout milli seconds to discover NAT.")]
        public int DiscoverTimeoutMilliSeconds { get; set; }

        [CommandLine.Option('c', "PortCandidates", Default = new ushort[] { }, Required = false,
            HelpText = "Candidates of port to map.")]
        public IEnumerable<ushort> PortCandidates { get; set; }

        [CommandLine.Option('u', "isPublic", Default = true, Required = false,
            HelpText = "Create public room if true.")]
        public bool IsPublic { get; set; }

        [CommandLine.Option('p', "password", Default = "", Required = false,
            HelpText = "A password for private room.")]
        public string Password { get; set; }
    }
}