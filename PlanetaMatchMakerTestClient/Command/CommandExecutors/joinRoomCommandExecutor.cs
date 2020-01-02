using System.IO;
using System.Threading;
using System.Threading.Tasks;

namespace PlanetaGameLabo.MatchMaker
{
    internal class JoinRoomCommandExecutor : StandardCommandExecutorBase<JoinRoomCommandOptions>
    {
        public override string Explanation => "Get host endpoint of room to join and close connection.";
        public override Command command => Command.JoinRoom;

        public JoinRoomCommandExecutor(StreamWriter outputStream) : base(outputStream)
        {
        }

        protected override async Task ExecuteStandardCommand(MatchMakerClient sharedClient,
            JoinRoomCommandOptions options,
            CancellationToken cancellationToken)
        {
            var gameHostEndPoint =
                await sharedClient.JoinRoomAsync(options.RoomGroupIndex, options.Id, options.Password);
            OutputStream.WriteLine($"Obtain game host endpoint of room: {gameHostEndPoint}");
            OutputStream.WriteLine("Connection will be closed.");
        }
    }

    internal class JoinRoomCommandOptions : StandardCommandOptions
    {
        [CommandLine.Value(0, MetaName = "RoomGroupIndex", Required = true,
            HelpText = "An index of room group where room is created.")]
        public byte RoomGroupIndex { get; set; }

        [CommandLine.Value(1, MetaName = "RoomID", Required = true,
            HelpText = "An room id you want to join.")]
        public uint Id { get; set; }

        [CommandLine.Option('p', "Password", Default = "", Required = false,
            HelpText = "A password for private room.")]
        public string Password { get; set; }
    }
}