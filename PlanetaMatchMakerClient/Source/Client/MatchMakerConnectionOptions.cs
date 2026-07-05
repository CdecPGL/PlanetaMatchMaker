using System;
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
        public MatchMakerConnectionOptions(MatchMakerConnectionMode mode = MatchMakerConnectionMode.Tls,
            string tlsTargetHost = null,
            RemoteCertificateValidationCallback remoteCertificateValidationCallback = null)
        {
            ValidateMode(mode);
            Mode = mode;
            TlsTargetHost = tlsTargetHost;
            RemoteCertificateValidationCallback = remoteCertificateValidationCallback;
        }

        public MatchMakerConnectionMode Mode { get; }

        public string TlsTargetHost { get; }

        public RemoteCertificateValidationCallback RemoteCertificateValidationCallback { get; }

        private static void ValidateMode(MatchMakerConnectionMode mode)
        {
            switch (mode)
            {
                case MatchMakerConnectionMode.Plain:
                case MatchMakerConnectionMode.Tls:
                    return;
                default:
                    throw new ArgumentOutOfRangeException(nameof(mode), mode, "Unsupported connection mode.");
            }
        }
    }
}
