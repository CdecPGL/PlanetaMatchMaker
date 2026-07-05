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
            TlsTargetHost = ParseTlsTargetHost(tlsTargetHost);
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

        private static string ParseTlsTargetHost(string tlsTargetHost)
        {
            if (string.IsNullOrEmpty(tlsTargetHost))
            {
                return null;
            }

            if (!MatchMakerAddress.TryParse(tlsTargetHost, out var parsedTlsTargetHost))
            {
                throw new ArgumentException("IPv4, IPv6 or host name is available.", nameof(tlsTargetHost));
            }

            return parsedTlsTargetHost.Value;
        }
    }
}
