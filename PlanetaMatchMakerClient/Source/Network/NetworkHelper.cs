using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
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

        /// <summary>
        /// Compare two IP addresses. This method considers an IPv4MappedToIPv6 address and a normal IPv4 address for same IP source are same.
        /// Default Equals() method does not so.
        /// Basically, there are no problem to use this method because the situation that an IPv4MappedToIPv6 address and a normal IPv4 address for same IP source indicates different endpoint is rare.
        /// However, if you want to strictly identify IP version in addition to the endpoint which IP address indicates, use Equals().
        /// </summary>
        /// <param name="left"></param>
        /// <param name="right"></param>
        /// <returns></returns>
        public static bool EqualsIpAddressSource(this IPAddress left, IPAddress right)
        {
            if (left == null || right == null)
            {
                return false;
            }

            if (left.IsIPv4MappedToIPv6 == right.IsIPv4MappedToIPv6)
            {
                return left.Equals(right);
            }

            return left.IsIPv4MappedToIPv6 ? left.MapToIPv4().Equals(right) : right.MapToIPv4().Equals(left);
        }
    }
}