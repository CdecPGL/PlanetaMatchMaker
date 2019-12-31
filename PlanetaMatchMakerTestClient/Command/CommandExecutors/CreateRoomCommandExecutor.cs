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
            await sharedClient.CreateRoomAsync(options.RoomGroupIndex, options.Name,
                options.MaxPlayerCount, options.PortNumber, options.IsPublic, options.Password);
            OutputStream.WriteLine(
                $"Room created with id \"{sharedClient.HostingRoomId}\" in room group {sharedClient.HostingRoomGroupIndex}.");
        }
    }

    internal class CreateRoomCommandOptions : StandardCommandOptions
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

        [CommandLine.Value(3, MetaName = "PortNumber", Required = true,
            HelpText = "A port number which is used in hosting game.")]
        public ushort PortNumber { get; set; }

        [CommandLine.Option('u', "isPublic", Default = true, Required = false,
            HelpText = "Create public room if true.")]
        public bool IsPublic { get; set; }

        [CommandLine.Option('p', "password", Default = "", Required = false,
            HelpText = "A password for private room.")]
        public string Password { get; set; }
    }
}