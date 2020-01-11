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
            var result = await sharedClient.CreateRoomWithCreatingPortMappingAsync(options.RoomGroupIndex, options.Name,
                options.MaxPlayerCount, options.Protocol, options.PortCandidates,
                options.DefaultPortNumber, options.DiscoverTimeoutMilliSeconds, options.IsPublic, options.Password);

            OutputStream.WriteLine(result.IsDefaultPortUsed
                ? "Default port is used."
                : $"Private port {result.UsedPrivatePortFromCandidates} and public port {result.UsedPublicPortFromCandidates} from candidates are used.");
            OutputStream.WriteLine(
                $"Room created with id \"{sharedClient.HostingRoomId}\" in room group {sharedClient.HostingRoomGroupIndex}.");
        }
    }

    internal class CreateRoomWithCreatingPortMappingCommandOptions : StandardCommandOptions
    {
        [CommandLine.Value(0, MetaName = "room_group_index", Required = true,
            HelpText = "An index of room group where room is created.")]
        public byte RoomGroupIndex { get; set; }

        [CommandLine.Value(1, MetaName = "room_name", Required = true,
            HelpText = "A name of room to create.")]
        public string Name { get; set; }

        [CommandLine.Value(2, MetaName = "max_player_count", Required = true,
            HelpText = "Max player count of room.")]
        public byte MaxPlayerCount { get; set; }

        [CommandLine.Value(3, MetaName = "protocol", Required = true,
            HelpText = "Transport protocol used to host game.")]
        public TransportProtocol Protocol { get; set; }

        [CommandLine.Value(4, MetaName = "default_port_number", Required = true,
            HelpText = "A default port number which is used in hosting game.")]
        public ushort DefaultPortNumber { get; set; }

        [CommandLine.Option('t', "discover_timeout_milli_seconds", Default = 5000, Required = false,
            HelpText = "Timeout milli seconds to discover NAT.")]
        public int DiscoverTimeoutMilliSeconds { get; set; }

        [CommandLine.Option('c', "port_candidates", Default = new ushort[] { }, Required = false, Separator = ',',
            HelpText = "Candidates of port to map.")]
        public IEnumerable<ushort> PortCandidates { get; set; }

        [CommandLine.Option('u', "is_public", Default = true, Required = false,
            HelpText = "Create public room if true.")]
        public bool IsPublic { get; set; }

        [CommandLine.Option('p', "password", Default = "", Required = false,
            HelpText = "A password for private room.")]
        public string Password { get; set; }
    }
}