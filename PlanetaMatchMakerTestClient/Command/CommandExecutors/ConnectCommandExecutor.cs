using System.IO;
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
            var connectionOptions = ConnectionOptionsFactory.Create(options.ConnectionMode, options.TlsTargetHost,
                options.AcceptInvalidTlsCertificate);
            var playerFullName =
                await sharedClient.ConnectAsync(new Host(options.ServerAddress),
                    new MatchMakerServerPort(options.ServerPort), new PlayerName(options.PlayerName),
                    connectionOptions);
            OutputStream.WriteLine($"Player Full Name: {playerFullName.GenerateFullName()}");
            OutputStream.WriteLine("Connect to the server successfully.");
        }
    }

    internal class ConnectCommandOptions : StandardCommandOptions
    {
        [CommandLine.Value(0, MetaName = "server_address", Required = true, HelpText = "An address of the server.")]
        public string ServerAddress { get; set; }

        [CommandLine.Value(1, MetaName = "server_port", Required = true, HelpText = "A port of the server.")]
        public ushort ServerPort { get; set; }

        [CommandLine.Value(2, MetaName = "player_name", Required = true, HelpText = "A name of player.")]
        public string PlayerName { get; set; }
    }
}
