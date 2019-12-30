using System.IO;
using System.Threading;
using System.Threading.Tasks;

namespace PlanetaGameLabo.MatchMaker
{
    internal class ListRoomGroupCommandExecutor : StandardCommandExecutorBase<StandardCommandOptions>
    {
        public override string Explanation => "List room groups.";
        public override Command command => Command.ListRoomGroup;

        public ListRoomGroupCommandExecutor(StreamWriter outputStream) : base(outputStream)
        {
        }

        protected override async Task ExecuteStandardCommand(MatchMakerClient sharedClient,
            StandardCommandOptions options,
            CancellationToken cancellationToken)
        {
            var results = await sharedClient.GetRoomGroupListAsync();
            OutputStream.WriteLine($"{results.Length} room groups found.");
            foreach (var result in results)
            {
                OutputStream.WriteLine($"    {result.Name}");
            }
        }
    }
}