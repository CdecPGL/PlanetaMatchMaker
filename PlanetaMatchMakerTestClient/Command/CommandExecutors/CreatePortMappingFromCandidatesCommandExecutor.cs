using System.Collections.Generic;
using System.IO;
using System.Threading;
using System.Threading.Tasks;

namespace PlanetaGameLabo.MatchMaker
{
    internal class
        CreatePortMappingFromCandidatesCommandExecutor : StandardCommandExecutorBase<
            CreatePortMappingFromCandidatesCommandOptions>
    {
        public override string Explanation =>
            "Create port mapping from port candidates to NAT which is compatible with UPnP or PMP.";

        public override Command command => Command.CreatePortMappingFromCandidates;

        public CreatePortMappingFromCandidatesCommandExecutor(StreamWriter outputStream) : base(outputStream)
        {
        }

        protected override async Task ExecuteStandardCommand(MatchMakerClient sharedClient,
            CreatePortMappingFromCandidatesCommandOptions options,
            CancellationToken cancellationToken)
        {
            var (privatePort, publicPort) = await sharedClient.PortMappingCreator.CreatePortMappingFromCandidates(
                options.Protocol, options.PrivatePortCandidates,
                options.PublicPortCandidates, options.Description);
            OutputStream.WriteLine(
                $"Port mapping is created to discovered NAT between private port \"{privatePort}\" and public port \"{publicPort}\".");
        }
    }

    internal class CreatePortMappingFromCandidatesCommandOptions : StandardCommandOptions
    {
        [CommandLine.Value(0, MetaName = "Protocol", Required = true,
            HelpText = "Transport protocol used to host game.")]
        public TransportProtocol Protocol { get; set; }

        [CommandLine.Value(1, MetaName = "PrivatePortCandidates", Required = true,
            HelpText = "Candidates of port to local network to map.")]
        public ICollection<ushort> PrivatePortCandidates { get; set; }

        [CommandLine.Value(2, MetaName = "PrivatePortCandidates", Required = true,
            HelpText = "Candidates of port to external network to map.")]
        public ICollection<ushort> PublicPortCandidates { get; set; }

        [CommandLine.Value(3, MetaName = "Description", Required = true,
            HelpText = "A Description of port mapping.")]
        public string Description { get; set; }
    }
}