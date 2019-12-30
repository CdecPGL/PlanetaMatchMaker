using System.IO;
using System.Threading;
using System.Threading.Tasks;

namespace PlanetaGameLabo.MatchMaker
{
    internal class CreatePortMappingCommandExecutor : StandardCommandExecutorBase<CreatePortMappingCommandOptions>
    {
        public override string Explanation => "Create port mapping to NAT which is compatible with UPnP or PMP.";
        public override Command command => Command.CreatePortMapping;

        public CreatePortMappingCommandExecutor(StreamWriter outputStream) : base(outputStream)
        {
        }

        protected override async Task ExecuteStandardCommand(MatchMakerClient sharedClient,
            CreatePortMappingCommandOptions options,
            CancellationToken cancellationToken)
        {
            await sharedClient.PortMappingCreator.CreatePortMapping(options.Protocol, options.PrivatePort,
                options.PublicPort, options.Description);
            OutputStream.WriteLine("Port mapping is created to discovered NAT.");
        }
    }

    internal class CreatePortMappingCommandOptions : StandardCommandOptions
    {
        [CommandLine.Value(0, MetaName = "Protocol", Required = true,
            HelpText = "Transport protocol used to host game.")]
        public TransportProtocol Protocol { get; set; }

        [CommandLine.Value(1, MetaName = "PrivatePort", Required = true,
            HelpText = "Port to local network to map.")]
        public ushort PrivatePort { get; set; }

        [CommandLine.Value(2, MetaName = "PublicPort", Required = true,
            HelpText = "Port to external network to map.")]
        public ushort PublicPort { get; set; }

        [CommandLine.Value(3, MetaName = "Description", Required = true,
            HelpText = "A Description of port mapping.")]
        public string Description { get; set; }
    }
}