using System.IO;
using System.Threading;
using System.Threading.Tasks;

namespace PlanetaGameLabo.MatchMaker
{
    internal class ShowPortMappingsCommandExecutor : StandardCommandExecutorBase<ShowPortMappingsCommandOptions>
    {
        public override string Explanation => "Show all port mappings registered in NAT which supports UPnP or PMP.";
        public override Command command => Command.ShowPortMappings;

        public ShowPortMappingsCommandExecutor(StreamWriter outputStream) : base(outputStream)
        {
        }

        protected override async Task ExecuteStandardCommand(MatchMakerClient sharedClient,
            ShowPortMappingsCommandOptions options,
            CancellationToken cancellationToken)
        {
            if (options.EnableForceToDiscoverNat | !sharedClient.PortMappingCreator.IsDiscoverNatDone)
            {
                OutputStream.WriteLine(" Execute discovering NAT device.");
                await sharedClient.PortMappingCreator.DiscoverNatAsync(options.DiscoverNatTimeoutMilliSeconds);
            }

            if (!sharedClient.PortMappingCreator.IsNatDeviceAvailable)
            {
                throw new CommandExecutionErrorException(
                    "There are no available NAT device found which supports UPnp or PMP.");
            }

            var portMappings = await sharedClient.PortMappingCreator.GetAllPortMappingsAsync();
            OutputStream.WriteLine($"{portMappings.Length} port mappings found.");
            foreach (var portMapping in portMappings)
            {
                OutputStream.WriteLine($"    {portMapping}");
            }
        }
    }

    internal class ShowPortMappingsCommandOptions : StandardCommandOptions
    {
        [CommandLine.Option('t', "discover_timeout", Default = 5000, Required = false,
            HelpText = "A timeout time by milliseconds to discover NAT Device.")]
        public int DiscoverNatTimeoutMilliSeconds { get; set; }

        [CommandLine.Option('f', "force_to_discover", Default = false, Required = false,
            HelpText = "Force to discover NAT device even if discover NAT device is already done.")]
        public bool EnableForceToDiscoverNat { get; set; }
    }
}