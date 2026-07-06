namespace PlanetaGameLabo.MatchMaker
{
    internal static class ConnectionOptionsFactory
    {
        public static ConnectionOptions Create(ConnectionMode connectionMode,
            string tlsTargetHost, bool acceptInvalidTlsCertificate)
        {
            return new ConnectionOptions(
                connectionMode,
                string.IsNullOrEmpty(tlsTargetHost) ? null : new Host(tlsTargetHost),
                acceptInvalidTlsCertificate
                    ? (sender, certificate, chain, sslPolicyErrors) => true
                    : null);
        }
    }
}
