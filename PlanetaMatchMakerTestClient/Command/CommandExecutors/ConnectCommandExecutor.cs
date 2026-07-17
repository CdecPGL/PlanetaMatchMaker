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
                    new ServerPort(options.ServerPort), new PlayerName(options.PlayerName),
                    AuthenticationOptionsFactory.CreateFromEnvironment(options.AuthenticationMethod,
                        options.AuthenticationCredentialEnvironmentVariable),
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

        [CommandLine.Option("connection_mode", Default = ConnectionMode.Tls, Required = false,
            HelpText = "Connection mode to the server. Plain or Tls.")]
        public ConnectionMode ConnectionMode { get; set; }

        [CommandLine.Option("tls_target_host", Default = "", Required = false,
            HelpText = "TLS target host used for server certificate validation.")]
        public string TlsTargetHost { get; set; }

        [CommandLine.Option("accept_invalid_tls_certificate", Default = false, Required = false,
            HelpText = "Accept invalid TLS certificates. Use only for development.")]
        public bool AcceptInvalidTlsCertificate { get; set; }

        [CommandLine.Option("authentication_method", Default = TestClientAuthenticationMethod.None, Required = false,
            HelpText = "Authentication method. None or Steam.")]
        public TestClientAuthenticationMethod AuthenticationMethod { get; set; }

        [CommandLine.Option("authentication_credential_environment_variable",
            Default = "PMMS_TEST_CLIENT_AUTHENTICATION_CREDENTIAL", Required = false,
            HelpText = "Environment variable containing a Steam ticket as a hex string. Not used for None.")]
        public string AuthenticationCredentialEnvironmentVariable { get; set; }
    }
}
