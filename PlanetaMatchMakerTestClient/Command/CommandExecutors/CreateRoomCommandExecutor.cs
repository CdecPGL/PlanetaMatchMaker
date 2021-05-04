using System.IO;
using System.Threading;
using System.Threading.Tasks;

namespace PlanetaGameLabo.MatchMaker
{
    internal class CreateRoomCommandExecutor : StandardCommandExecutorBase<CreateRoomCommandOptions>
    {
        public override string Explanation => "Create room to indicated room group.";
        public override Command command => Command.CreateRoom;

        public CreateRoomCommandExecutor(StreamWriter outputStream) : base(outputStream)
        {
        }

        protected override async Task ExecuteStandardCommand(MatchMakerClient sharedClient,
            CreateRoomCommandOptions options,
            CancellationToken cancellationToken)
        {
            await sharedClient.CreateRoomAsync(options.MaxPlayerCount, options.PortNumber, options.Password);
            OutputStream.WriteLine($"Room created with id \"{sharedClient.HostingRoomId}\".");
        }
    }

    internal class CreateRoomCommandOptions : StandardCommandOptions
    {
        [CommandLine.Value(1, MetaName = "max_player_count", Required = true,
            HelpText = "Max player count of room.")]
        public byte MaxPlayerCount { get; set; }

        [CommandLine.Value(2, MetaName = "port_number", Required = true,
            HelpText = "A port number which is used in hosting game.")]
        public ushort PortNumber { get; set; }

        [CommandLine.Option('p', "password", Default = "", Required = false,
            HelpText = "A password for private room.")]
        public string Password { get; set; }
    }
}