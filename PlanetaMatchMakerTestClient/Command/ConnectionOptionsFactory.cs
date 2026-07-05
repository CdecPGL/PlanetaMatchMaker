namespace PlanetaGameLabo.MatchMaker
{
    internal static class ConnectionOptionsFactory
    {
        public static ConnectionOptions Create(string connectionMode,
            string tlsTargetHost, bool acceptInvalidTlsCertificate)
        {
            return new ConnectionOptions(
                ParseConnectionMode(connectionMode),
                string.IsNullOrEmpty(tlsTargetHost) ? null : new Host(tlsTargetHost),
                acceptInvalidTlsCertificate
                    ? (sender, certificate, chain, sslPolicyErrors) => true
                    : null);
        }

        private static ConnectionMode ParseConnectionMode(string connectionMode)
        {
            switch (connectionMode?.Trim().ToLowerInvariant())
            {
                case "plain":
                    return ConnectionMode.Plain;
                case "tls":
                    return ConnectionMode.Tls;
                default:
                    throw new CommandExecutionErrorException(
                        $"{nameof(connectionMode)} must be plain or tls. ({nameof(connectionMode)}: {connectionMode})");
            }
        }
    }
}
