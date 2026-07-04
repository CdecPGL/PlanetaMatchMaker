using System.Net.Security;

namespace PlanetaGameLabo.MatchMaker
{
    public enum MatchMakerConnectionMode
    {
        Plain,
        Tls
    }

    public sealed class MatchMakerConnectionOptions
    {
        public MatchMakerConnectionMode Mode { get; set; } = MatchMakerConnectionMode.Tls;

        public string TlsTargetHost { get; set; }

        public RemoteCertificateValidationCallback RemoteCertificateValidationCallback { get; set; }
    }
}
