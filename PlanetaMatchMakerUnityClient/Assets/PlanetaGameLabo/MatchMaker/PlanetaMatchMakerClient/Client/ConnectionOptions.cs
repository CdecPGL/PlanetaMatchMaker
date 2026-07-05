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
            Host tlsTargetHost = null,
            RemoteCertificateValidationCallback remoteCertificateValidationCallback = null)
        {
            ValidateMode(mode);
            Mode = mode;
            TlsTargetHost = tlsTargetHost;
            RemoteCertificateValidationCallback = remoteCertificateValidationCallback;
        }

        public ConnectionMode Mode { get; }

        public Host TlsTargetHost { get; }

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
    }
}
