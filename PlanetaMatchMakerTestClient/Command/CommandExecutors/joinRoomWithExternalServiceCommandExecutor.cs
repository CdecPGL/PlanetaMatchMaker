using System.IO;
using System.Threading;
using System.Threading.Tasks;

namespace PlanetaGameLabo.MatchMaker
{
    internal class
        JoinRoomWithExternalServiceCommandExecutor : StandardCommandExecutorBase<
            JoinRoomWithExternalServiceCommandOptions>
    {
        public override string Explanation => "Get host info of room to join with external service and close connection.";
        public override Command command => Command.JoinRoomWithExternalService;

        public JoinRoomWithExternalServiceCommandExecutor(StreamWriter outputStream) : base(outputStream)
        {
        }

        protected override async Task ExecuteStandardCommand(MatchMakerClient sharedClient,
            JoinRoomWithExternalServiceCommandOptions options,
            CancellationToken cancellationToken)
        {
            var result =
                await sharedClient.JoinRoomWithExternalServiceAsync(options.Id, options.ConnectionEstablishMode,
                    new RoomPassword(options.Password));
            OutputStream.WriteLine($"Obtain game host signaling mode: {result.ConnectionEstablishMode}");
            OutputStream.WriteLine(
                $"Obtain game host P2P service peer ID: {result.P2pServicePeerId.Value}");
            OutputStream.WriteLine("Connection will be closed.");
        }
    }

    internal class JoinRoomWithExternalServiceCommandOptions : StandardCommandOptions
    {
        [CommandLine.Value(1, MetaName = "room_id", Required = true,
            HelpText = "An room id you want to join.")]
        public uint Id { get; set; }

        [CommandLine.Value(2, MetaName = "connection_establish _mode", Required = true,
            HelpText = "a way how clients establish connection to the host.")]
        public GameHostConnectionEstablishMode ConnectionEstablishMode { get; set; }

        [CommandLine.Option('p', "password", Default = "", Required = false,
            HelpText = "A password for private room.")]
        public string Password { get; set; }
    }
}
