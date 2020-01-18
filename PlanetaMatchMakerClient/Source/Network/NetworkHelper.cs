using System;
using System.Collections.Generic;
using System.Linq;
using System.Net.NetworkInformation;

namespace PlanetaGameLabo.MatchMaker
{
    internal static class NetworkHelper
    {
        /// <summary>
        /// Check port availability.
        /// The port is considered as not available if it is used in this machine by indicated protocol.
        /// </summary>
        /// <param name="protocol"></param>
        /// <param name="port"></param>
        /// <returns></returns>
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

        /// <summary>
        /// Filter ports by its availability.
        /// The port is considered as not available if it is used in this machine by indicated protocol.
        /// </summary>
        /// <param name="protocol"></param>
        /// <param name="ports"></param>
        /// <returns></returns>
        public static IEnumerable<ushort> FilterPortsByAvailability(TransportProtocol protocol,
            IEnumerable<ushort> ports)
        {
            var ipGlobalProperties = IPGlobalProperties.GetIPGlobalProperties();
            HashSet<ushort> usedPorts;
            switch (protocol)
            {
                case TransportProtocol.Tcp:
                    usedPorts = new HashSet<ushort>(ipGlobalProperties.GetActiveTcpConnections()
                        .Select(c => (ushort)c.LocalEndPoint.Port));
                    break;
                case TransportProtocol.Udp:
                    usedPorts = new HashSet<ushort>(ipGlobalProperties.GetActiveUdpListeners()
                        .Select(l => (ushort)l.Port));
                    break;
                default:
                    throw new ArgumentOutOfRangeException(nameof(protocol), protocol, null);
            }

            return ports.Where(p => !usedPorts.Contains(p));
        }
    }
}