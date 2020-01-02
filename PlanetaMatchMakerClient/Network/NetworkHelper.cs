using System;
using System.Linq;
using System.Net.NetworkInformation;

namespace PlanetaGameLabo.MatchMaker
{
    internal static class NetworkHelper
    {
        public static bool CheckPortAvailability(TransportProtocol protocol, ushort port)
        {
            var ipGlobalProperties = IPGlobalProperties.GetIPGlobalProperties();
            switch (protocol)
            {
                case TransportProtocol.Tcp:
                    return ipGlobalProperties.GetActiveTcpConnections().All(c => c.LocalEndPoint.Port != port);
                case TransportProtocol.Udp:
                    return ipGlobalProperties.GetActiveUdpListeners().All(l => l.Port != port);
                default:
                    throw new ArgumentOutOfRangeException(nameof(protocol), protocol, null);
            }
        }
    }
}