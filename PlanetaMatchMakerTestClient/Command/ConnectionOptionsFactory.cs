namespace PlanetaGameLabo.MatchMaker
{
    internal static class ConnectionOptionsFactory
    {
        public static MatchMakerConnectionOptions Create(string connectionMode,
            string tlsTargetHost, bool acceptInvalidTlsCertificate)
        {
            return new MatchMakerConnectionOptions
            {
                Mode = ParseConnectionMode(connectionMode),
                TlsTargetHost = string.IsNullOrEmpty(tlsTargetHost) ? null : tlsTargetHost,
                RemoteCertificateValidationCallback = acceptInvalidTlsCertificate
                    ? (sender, certificate, chain, sslPolicyErrors) => true
                    : null
            };
        }

        private static MatchMakerConnectionMode ParseConnectionMode(string connectionMode)
        {
            switch (connectionMode?.Trim().ToLowerInvariant())
            {
                case "plain":
                    return MatchMakerConnectionMode.Plain;
                case "tls":
                    return MatchMakerConnectionMode.Tls;
                default:
                    throw new CommandExecutionErrorException(
                        $"{nameof(connectionMode)} must be plain or tls. ({nameof(connectionMode)}: {connectionMode})");
            }
        }
    }
}
