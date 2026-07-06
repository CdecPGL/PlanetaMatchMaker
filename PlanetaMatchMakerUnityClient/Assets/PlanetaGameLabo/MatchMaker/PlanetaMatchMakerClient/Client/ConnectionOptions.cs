using System;
using System.Net.Security;

namespace PlanetaGameLabo.MatchMaker
{
    public enum ConnectionMode
    {
        Plain,
        Tls
    }

    public readonly struct ConnectionOptions : IEquatable<ConnectionOptions>
    {
        public ConnectionOptions(ConnectionMode mode = ConnectionMode.Tls,
            Host? tlsTargetHost = null,
            RemoteCertificateValidationCallback remoteCertificateValidationCallback = null)
        {
            ValidateMode(mode);
            this.mode = mode;
            this.tlsTargetHost = tlsTargetHost;
            this.remoteCertificateValidationCallback = remoteCertificateValidationCallback;
        }

        private readonly ConnectionMode? mode;

        private readonly Host? tlsTargetHost;

        private readonly RemoteCertificateValidationCallback remoteCertificateValidationCallback;

        public ConnectionMode Mode => mode ?? ConnectionMode.Tls;

        public Host? TlsTargetHost => tlsTargetHost;

        public RemoteCertificateValidationCallback RemoteCertificateValidationCallback => remoteCertificateValidationCallback;

        public bool Equals(ConnectionOptions other)
        {
            return Mode == other.Mode &&
                   TlsTargetHost.Equals(other.TlsTargetHost) &&
                   Equals(RemoteCertificateValidationCallback, other.RemoteCertificateValidationCallback);
        }

        public static bool operator ==(ConnectionOptions left, ConnectionOptions right)
        {
            return left.Equals(right);
        }

        public static bool operator !=(ConnectionOptions left, ConnectionOptions right)
        {
            return !left.Equals(right);
        }

        public override bool Equals(object obj)
        {
            return obj is ConnectionOptions other && Equals(other);
        }

        public override int GetHashCode()
        {
            unchecked
            {
                var hash = 17;
                hash = hash * 31 + Mode.GetHashCode();
                hash = hash * 31 + TlsTargetHost.GetHashCode();
                hash = hash * 31 + (RemoteCertificateValidationCallback == null
                    ? 0
                    : RemoteCertificateValidationCallback.GetHashCode());
                return hash;
            }
        }

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
