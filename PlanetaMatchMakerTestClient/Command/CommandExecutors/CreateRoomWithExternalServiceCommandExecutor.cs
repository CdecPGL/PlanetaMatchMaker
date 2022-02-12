using System.IO;
using System.Threading;
using System.Threading.Tasks;

namespace PlanetaGameLabo.MatchMaker
{
    internal class CreateRoomWithExternalServiceCommandExecutor : StandardCommandExecutorBase<CreateRoomWithExternalServiceCommandOptions>
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
            await sharedClient.CreateRoomWithExternalServiceAsync(options.MaxPlayerCount, options.ExternalId, options.Password);
            OutputStream.WriteLine($"Room created with id \"{sharedClient.HostingRoomId}\".");
        }
    }

    internal class CreateRoomWithExternalServiceCommandOptions : StandardCommandOptions
    {
        [CommandLine.Value(1, MetaName = "max_player_count", Required = true,
            HelpText = "Max player count of room.")]
        public byte MaxPlayerCount { get; set; }

        [CommandLine.Value(2, MetaName = "external_id", Required = true,
            HelpText = "An ID of eternal service for signaling.")]
        public string ExternalId { get; set; }

        [CommandLine.Option('s', "password", Default = "", Required = false,
            HelpText = "A password for private room.")]
        public string Password { get; set; }
    }
}