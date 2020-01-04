using System.IO;
using System.Threading;
using System.Threading.Tasks;

namespace PlanetaGameLabo.MatchMaker
{
    internal class ReleasePortMappingsCommandExecutor : StandardCommandExecutorBase<StandardCommandOptions>
    {
        public override string Explanation => "Release port mappings created by this client.";
        public override Command command => Command.ReleasePortMappings;

        public ReleasePortMappingsCommandExecutor(StreamWriter outputStream) : base(outputStream)
        {
        }

        protected override async Task ExecuteStandardCommand(MatchMakerClient sharedClient,
            StandardCommandOptions options,
            CancellationToken cancellationToken)
        {
            NatPortMappingCreator.ReleaseCreatedPortMappings();
            OutputStream.WriteLine("Port mappings are released.");
            OutputStream.WriteLine("Note: this command doesn't check errors.");
        }
    }
}