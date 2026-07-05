using System;
using System.Net.Security;

namespace PlanetaGameLabo.MatchMaker
{
    public enum ConnectionMode
    {
        Plain,
        Tls
    }

    public sealed class ConnectionOptions
    {
        public ConnectionOptions(ConnectionMode mode = ConnectionMode.Tls,
            string tlsTargetHost = null,
            RemoteCertificateValidationCallback remoteCertificateValidationCallback = null)
        {
            ValidateMode(mode);
            Mode = mode;
            TlsTargetHost = ParseTlsTargetHost(tlsTargetHost);
            RemoteCertificateValidationCallback = remoteCertificateValidationCallback;
        }

        public ConnectionMode Mode { get; }

        public string TlsTargetHost { get; }

        public RemoteCertificateValidationCallback RemoteCertificateValidationCallback { get; }

        private static void ValidateMode(ConnectionMode mode)
        {
            switch (mode)
            {
                case ConnectionMode.Plain:
                case ConnectionMode.Tls:
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

            if (!NetworkServerAddress.TryParse(tlsTargetHost, out var parsedTlsTargetHost))
            {
                throw new ArgumentException("IPv4, IPv6 or host name is available.", nameof(tlsTargetHost));
            }

            return parsedTlsTargetHost.Value;
        }
    }
}
