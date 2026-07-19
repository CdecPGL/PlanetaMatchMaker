using System.IO;
using System.Threading;
using System.Threading.Tasks;

namespace PlanetaGameLabo.MatchMaker
{
    internal class
        CreateRoomWithExternalServiceCommandExecutor : StandardCommandExecutorBase<
            CreateRoomWithExternalServiceCommandOptions>
    {
        public override string Explanation => "Create a room with external service ID.";
        public override Command command => Command.CreateRoomWithExternalService;

        public CreateRoomWithExternalServiceCommandExecutor(StreamWriter outputStream) : base(outputStream)
        {
        }

        protected override async Task ExecuteStandardCommand(MatchMakerClient sharedClient,
            CreateRoomWithExternalServiceCommandOptions options,
            CancellationToken cancellationToken)
        {
            var peerId = string.IsNullOrEmpty(options.P2pServicePeerId)
                ? (P2pServicePeerId?)null
                : new P2pServicePeerId(options.P2pServicePeerId);
            await sharedClient.CreateRoomWithExternalServiceAsync(options.MaxPlayerCount,
                options.ConnectionEstablishMode, peerId,
                new RoomPassword(options.Password));
            OutputStream.WriteLine($"Room created with id \"{sharedClient.HostingRoomId}\".");
        }
    }

    internal class CreateRoomWithExternalServiceCommandOptions : StandardCommandOptions
    {
        [CommandLine.Value(1, MetaName = "max_player_count", Required = true,
            HelpText = "Max player count of room.")]
        public byte MaxPlayerCount { get; set; }

        [CommandLine.Value(2, MetaName = "connection_establish_mode", Required = true,
            HelpText = "A way how clients connect to the host.")]
        public GameHostConnectionEstablishMode ConnectionEstablishMode { get; set; }

        [CommandLine.Value(3, MetaName = "p2p_service_peer_id", Required = false,
            HelpText = "A peer ID for an other P2P service. Omit for Steam.")]
        public string P2pServicePeerId { get; set; }

        [CommandLine.Option('s', "password", Default = "", Required = false,
            HelpText = "A password for private room.")]
        public string Password { get; set; }
    }
}
