using System.IO;
using System.Linq;
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
            var roomEndpoint =
                await sharedClient.JoinRoomAsync(options.Id, options.Password);
            OutputStream.WriteLine($"Obtain game host signaling mode: {roomEndpoint.SignalingMethod}");
            OutputStream.WriteLine($"Obtain game host endpoint of room: {roomEndpoint.IPEndpoint}");
            OutputStream.WriteLine($"Obtain game host external id of room: {string.Join("",roomEndpoint.ExternalId.Select(b=>$"{b:X00}"))} ({roomEndpoint.ExternalIdString})");
            OutputStream.WriteLine("Connection will be closed.");
        }
    }

    internal class JoinRoomCommandOptions : StandardCommandOptions
    {
        [CommandLine.Value(1, MetaName = "room_id", Required = true,
            HelpText = "An room id you want to join.")]
        public uint Id { get; set; }

        [CommandLine.Option('p', "password", Default = "", Required = false,
            HelpText = "A password for private room.")]
        public string Password { get; set; }
    }
}