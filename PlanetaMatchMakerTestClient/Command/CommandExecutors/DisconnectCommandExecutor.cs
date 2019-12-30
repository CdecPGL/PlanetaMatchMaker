using System.IO;
using System.Threading;
using System.Threading.Tasks;

namespace PlanetaGameLabo.MatchMaker
{
    internal class DisconnectCommandExecutor : StandardCommandExecutorBase<StandardCommandOptions>
    {
        public override string Explanation => "Disconnect from connecting server.";
        public override Command command => Command.Disconnect;

        public DisconnectCommandExecutor(StreamWriter outputStream) : base(outputStream)
        {
        }

        protected override async Task ExecuteStandardCommand(MatchMakerClient sharedClient,
            StandardCommandOptions options,
            CancellationToken cancellationToken)
        {
            sharedClient.Close();
        }
    }
}