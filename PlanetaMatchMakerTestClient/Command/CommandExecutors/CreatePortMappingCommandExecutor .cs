using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Threading;
using System.Threading.Tasks;

namespace PlanetaGameLabo.MatchMaker
{
    internal class CreatePortMappingCommandExecutor : StandardCommandExecutorBase<CreatePortMappingCommandOptions>
    {
        public override string Explanation => "Create port mapping to NAT which supports UPnP or PMP.";
        public override Command command => Command.CreatePortMapping;

        public CreatePortMappingCommandExecutor(StreamWriter outputStream) : base(outputStream)
        {
        }

        protected override async Task ExecuteStandardCommand(MatchMakerClient sharedClient,
            CreatePortMappingCommandOptions options,
            CancellationToken cancellationToken)
        {
            if (options.EnableForceToDiscoverNat | !sharedClient.PortMappingCreator.IsDiscoverNatDone)
            {
                OutputStream.WriteLine(" Execute discovering NAT device.");
                await sharedClient.PortMappingCreator.DiscoverNat(options.DiscoverNatTimeoutMilliSeconds);
            }

            if (!sharedClient.PortMappingCreator.IsNatDeviceAvailable)
            {
                throw new CommandExecutionErrorException(
                    "There are no available NAT device found which supports UPnp or PMP.");
            }

            if (!options.PrivatePortCandidates.Any() && !options.PublicPortCandidates.Any())
            {
                await sharedClient.PortMappingCreator.CreatePortMapping(options.Protocol, options.PrivatePort,
                    options.PublicPort, options.Description);
                OutputStream.WriteLine("Port mapping is created to discovered NAT.");
            }
            else
            {
                var privatePortCandidates = new HashSet<ushort>() {options.PrivatePort};
                if (options.PrivatePortCandidates != null)
                {
                    foreach (var privatePortCandidate in options.PrivatePortCandidates)
                    {
                        privatePortCandidates.Add(privatePortCandidate);
                    }
                }

                var publicPortCandidates = new HashSet<ushort>() {options.PublicPort};
                if (options.PublicPortCandidates != null)
                {
                    foreach (var publicPortCandidate in options.PublicPortCandidates)
                    {
                        publicPortCandidates.Add(publicPortCandidate);
                    }
                }

                var (privatePort, publicPort) =
                    await sharedClient.PortMappingCreator.CreatePortMappingFromCandidates(options.Protocol,
                        privatePortCandidates, publicPortCandidates, options.Description);
                OutputStream.WriteLine(
                    $"Port mapping is created to discovered NAT between private port \"{privatePort}\" and public port \"{publicPort}\".");
            }
        }
    }

    internal class CreatePortMappingCommandOptions : StandardCommandOptions
    {
        [CommandLine.Value(0, MetaName = "Protocol", Required = true,
            HelpText = "Transport protocol used for hosting game.")]
        public TransportProtocol Protocol { get; set; }

        [CommandLine.Value(1, MetaName = "PrivatePort", Required = true,
            HelpText = "Port to local network to map.")]
        public ushort PrivatePort { get; set; }

        [CommandLine.Value(2, MetaName = "PublicPort", Required = true,
            HelpText = "Port to external network to map.")]
        public ushort PublicPort { get; set; }

        [CommandLine.Option('d', "Description", Default = "PlanetaMatchMakerTestClient", Required = false,
            HelpText = "A Description of port mapping.")]
        public string Description { get; set; }

        [CommandLine.Option('t', "discoverTimeout", Default = 5000, Required = false,
            HelpText = "A timeout time by milliseconds to discover NAT Device.")]
        public int DiscoverNatTimeoutMilliSeconds { get; set; }

        [CommandLine.Option('f', "forceToDiscover", Default = false, Required = false,
            HelpText = "Force to discover NAT device even if discover NAT device is already done.")]
        public bool EnableForceToDiscoverNat { get; set; }

        [CommandLine.Option('r', "PrivatePortCandidates", Default = new ushort[] { }, Required = false, Separator = ',',
            HelpText = "Candidates of private port to local network to map.")]
        public IEnumerable<ushort> PrivatePortCandidates { get; set; }

        [CommandLine.Option('u', "PublicPortCandidates", Default = new ushort[] { }, Required = false, Separator = ',',
            HelpText = "Candidates of public port to external network to map.")]
        public IEnumerable<ushort> PublicPortCandidates { get; set; }
    }
}