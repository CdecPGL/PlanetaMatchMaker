using System.IO;
using System.Threading;
using System.Threading.Tasks;

namespace PlanetaGameLabo.MatchMaker
{
    internal class
        UpdateHostingRoomStatusCommandExecutor : StandardCommandExecutorBase<UpdateHostingRoomStatusCommandOptions>
    {
        public override string Explanation => "Update hosting room status.";
        public override Command command => Command.UpdateHostingRoomStatus;

        public UpdateHostingRoomStatusCommandExecutor(StreamWriter outputStream) : base(outputStream)
        {
        }

        protected override async Task ExecuteStandardCommand(MatchMakerClient sharedClient,
            UpdateHostingRoomStatusCommandOptions options,
            CancellationToken cancellationToken)
        {
            if (options.PlayerCount == null)
            {
                await sharedClient.UpdateHostingRoomStatusAsync(options.Status);
            }
            else
            {
                await sharedClient.UpdateHostingRoomStatusAsync(options.Status, true, options.PlayerCount.Value);
            }

            OutputStream.WriteLine(
                $"Status of hosting room (ID: {sharedClient.HostingRoomId}, Group: {sharedClient.HostingRoomGroupIndex}) is changed to \"{options.Status}\".");
            if (options.PlayerCount != null)
            {
                OutputStream.WriteLine(
                    $"Player count of hosting room (ID: {sharedClient.HostingRoomId}, Group: {sharedClient.HostingRoomGroupIndex}) is changed to \"{options.PlayerCount}\".");
            }

            OutputStream.WriteLine("Error in the server is not noticed because this API doesn't return reply.");
        }
    }

    internal class UpdateHostingRoomStatusCommandOptions : StandardCommandOptions
    {
        [CommandLine.Value(0, MetaName = "RoomStatus", Required = true,
            HelpText = "new status.")]
        public RoomStatus Status { get; set; }

        [CommandLine.Option('c', "PlayerCount", Default = null, Required = false,
            HelpText = "New player count.")]
        public byte? PlayerCount { get; set; }
    }
}