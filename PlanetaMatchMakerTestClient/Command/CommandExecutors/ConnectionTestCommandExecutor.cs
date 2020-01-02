using System;
using System.IO;
using System.Threading;
using System.Threading.Tasks;

namespace PlanetaGameLabo.MatchMaker
{
    internal class ConnectionTestCommandExecutor : StandardCommandExecutorBase<ConnectionTestCommandOptions>
    {
        public override string Explanation => "Check if this machine is connectable from internet to host game.";
        public override Command command => Command.ConnectionTest;

        public ConnectionTestCommandExecutor(StreamWriter outputStream) : base(outputStream)
        {
        }

        protected override async Task ExecuteStandardCommand(MatchMakerClient sharedClient,
            ConnectionTestCommandOptions options,
            CancellationToken cancellationToken)
        {
            try
            {
                var succeed = await sharedClient.ConnectionTestAsync(options.Protocol, options.PortNumber);
                var succeedText = succeed ? "succeed" : "failed";
                OutputStream.WriteLine($"Connection test is {succeedText}.");
            }
            catch (InvalidOperationException e)
            {
                throw new CommandExecutionErrorException(e.Message);
            }
        }
    }

    internal class ConnectionTestCommandOptions : StandardCommandOptions
    {
        [CommandLine.Value(0, MetaName = "Protocol", Required = true,
            HelpText = "Transport protocol used to host game.")]
        public TransportProtocol Protocol { get; set; }

        [CommandLine.Value(1, MetaName = "Port", Required = true,
            HelpText = "Port used to host game.")]
        public ushort PortNumber { get; set; }
    }
}