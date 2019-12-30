using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace PlanetaGameLabo.MatchMaker
{
    internal class ConnectCommandExecutor : StandardCommandExecutorBase<ConnectCommandOptions>
    {
        public override string Explanation => "Connect to a server.";
        public override Command command => Command.Connect;

        public ConnectCommandExecutor(StreamWriter outputStream) : base(outputStream)
        {
        }

        protected override async Task ExecuteStandardCommand(MatchMakerClient sharedClient,
            ConnectCommandOptions options,
            CancellationToken cancellationToken)
        {
            if (sharedClient.Connected)
            {
                throw new CommandExecutionErrorException("Already connected.");
            }

            await sharedClient.ConnectAsync(options.ServerAddress, options.ServerPort);
        }
    }

    internal class ConnectCommandOptions : StandardCommandOptions
    {
        [CommandLine.Value(0, MetaName = "ServerAddress", Required = true, HelpText = "An address of the server.")]
        public string ServerAddress { get; set; }

        [CommandLine.Value(1, MetaName = "ServerPort", Required = true, HelpText = "A port of the server.")]
        public ushort ServerPort { get; set; }
    }
}