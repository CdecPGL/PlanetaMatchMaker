using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Net.NetworkInformation;
using System.Net.Sockets;
using System.Threading;
using System.Threading.Tasks;
using Open.Nat;

#pragma warning disable CA1303
namespace PlanetaGameLabo.MatchMaker
{
    /// <summary>
    /// A class to create port mapping to NAT device which is compatible with UPnP or PMP.
    /// </summary>
    public sealed class NatPortMappingCreator
    {
        public struct PortMapping
        {
            public readonly TransportProtocol Protocol;
            public readonly IPAddress PublicIpAddress;
            public readonly ushort PublicPort;
            public readonly IPAddress PrivateIpAddress;
            public readonly ushort PrivatePort;

            public PortMapping(TransportProtocol protocol, IPAddress publicIpAddress, ushort publicPort,
                IPAddress privateIpAddress, ushort privatePort)
            {
                Protocol = protocol;
                PublicIpAddress = publicIpAddress;
                PublicPort = publicPort;
                PrivateIpAddress = privateIpAddress;
                PrivatePort = privatePort;
            }

            public override string ToString()
            {
                return
                    $"PortMapping(Protocol: {Protocol}, PrivateEndPoint: {PrivateIpAddress}:{PrivatePort}, PublicEndPoint: {PublicIpAddress}:{PublicPort})";
            }
        }

        /// <summary>
        /// true if NAT there are available NAT device.
        /// </summary>
        public bool IsNatDeviceAvailable { get; private set; }

        /// <summary>
        /// true if DiscoverNatAsync is executed at least once.
        /// </summary>
        public bool IsDiscoverNatDone { get; private set; }

        public ILogger Logger { get; }

        public NatPortMappingCreator(ILogger logger)
        {
            Logger = logger;
        }

        /// <summary>
        /// Get all port mappings.
        /// </summary>
        /// <exception cref="InvalidOperationException">DiscoverNatAsync is not executed or NAT device is not available</exception>
        /// <exception cref="ClientErrorException">Failed to get port mappings</exception>
        /// <returns></returns>
        public async Task<PortMapping[]> GetAllPortMappingsAsync()
        {
            if (!IsDiscoverNatDone)
            {
                throw new InvalidOperationException("DiscoverNatAsync should be executed before this method.");
            }

            if (!IsNatDeviceAvailable)
            {
                throw new InvalidOperationException("NAT device is not available.");
            }

            try
            {
                var mappings = await natDevice.GetAllMappingsAsync().ConfigureAwait(false);
                return mappings.Select(m =>
                    new PortMapping(m.Protocol == Protocol.Tcp ? TransportProtocol.Tcp : TransportProtocol.Udp,
                        m.PublicIP, (ushort)m.PublicPort, m.PrivateIP, (ushort)m.PrivatePort)).ToArray();
            }
            catch (MappingException e)
            {
                throw new ClientErrorException(ClientErrorCode.UnknownError, e.Message);
            }
        }

        /// <summary>
        /// Create port mapping.
        /// </summary>
        /// <param name="protocol"></param>
        /// <param name="privatePort"></param>
        /// <param name="publicPort"></param>
        /// <param name="description"></param>
        /// <exception cref="InvalidOperationException">DiscoverNatAsync is not executed or NAT device is not available</exception>
        /// <exception cref="ClientErrorException">Failed to create port mapping</exception>
        /// <returns></returns>
        public async Task CreatePortMappingAsync(TransportProtocol protocol, ushort privatePort, ushort publicPort,
            string description = "PlanetaMatchMakerClient")
        {
            if (!IsDiscoverNatDone)
            {
                throw new InvalidOperationException("DiscoverNatAsync should be executed before this method.");
            }

            if (!IsNatDeviceAvailable)
            {
                throw new InvalidOperationException("NAT device is not available.");
            }

            try
            {
                // Make port mapping lifetime type session
                await natDevice.CreatePortMapAsync(new Mapping(
                    protocol == TransportProtocol.Tcp ? Protocol.Tcp : Protocol.Udp, privatePort, publicPort,
                    int.MaxValue, description)).ConfigureAwait(false);
                Logger.Log(LogLevel.Info,
                    $"Port Mapping (Protocol: {protocol}, PrivatePort: {privatePort}, PublicPort: {publicPort}) is Created.");
            }
            catch (MappingException e)
            {
                throw new ClientErrorException(ClientErrorCode.CreatingPortMappingFailed, e.Message);
            }
        }

        /// <summary>
        /// Create port mapping by select available port pair from candidate.
        /// If there are port mapping which matches port candidates, we reuse it.
        /// If not, we find port pair which is not used and create port mapping with the pair.
        /// This method doesn't consider if port is used in this computer by indicated protocol.
        /// </summary>
        /// <param name="protocol"></param>
        /// <param name="privatePortCandidates"></param>
        /// <param name="publicPortCandidates"></param>
        /// <param name="description"></param>
        /// <exception cref="InvalidOperationException">DiscoverNatAsync is not executed or NAT device is not available</exception>
        /// <exception cref="ClientErrorException">Failed to create port mapping. There may be not available port pair in the candidates</exception>
        /// <returns></returns>
        public async Task<(ushort privatePort, ushort publicPort)> CreatePortMappingFromCandidatesAsync(
            TransportProtocol protocol,
            ICollection<ushort> privatePortCandidates,
            ICollection<ushort> publicPortCandidates, string description = "PlanetaMatchMakerClient")
        {
            if (!IsDiscoverNatDone)
            {
                throw new InvalidOperationException("DiscoverNatAsync should be executed before this method.");
            }

            if (!IsNatDeviceAvailable)
            {
                throw new InvalidOperationException("NAT device is not available.");
            }

            string PortMappingToString(Mapping mapping)
            {
                return
                    $"{mapping.PublicIP}:{mapping.PublicPort} to {mapping.PrivateIP}:{mapping.PrivatePort}({mapping.Protocol})";
            }

            Logger.Log(LogLevel.Info, $"Private port candidates are [{string.Join(",", privatePortCandidates)}].");
            Logger.Log(LogLevel.Info, $"Public port candidates are [{string.Join(",", publicPortCandidates)}].");

            var proto = protocol == TransportProtocol.Tcp ? Protocol.Tcp : Protocol.Udp;
            var hostname = Dns.GetHostName();

            // Select IP address of this computer whose network is same as it of NAT device
            var myAddressesWithPrefixLength = GetAllIpAddressesWithMask();
            var natDeviceAddress = natDevice.LocalAddress;
            var hostAddresses = myAddressesWithPrefixLength.Where(ap =>
            {
                if (ap.IpAddress.AddressFamily != natDeviceAddress.AddressFamily)
                {
                    return false;
                }

                // Search this host IP address which is in the network the NAT device in by comparing prefix of IP address.
                // PrefixLength is available in both IPv4 and IPv6, so below code is capable of both IPv4 and IPv6.
                var natDeviceAddressBytes = natDeviceAddress.GetAddressBytes();
                var maskBytes = Enumerable.Repeat<byte>(0, natDeviceAddressBytes.Length).ToArray();
                for (var i = 0; i < ap.PrefixLength / 8; ++i)
                {
                    if (i == ap.PrefixLength / 8 - 1)
                    {
                        var rest = ap.PrefixLength % 8;
                        maskBytes[i] = (byte)~(2 ^ (8 - rest) - 1);
                    }
                    else
                    {
                        maskBytes[i] = 0b11111111;
                    }
                }

                var hostAddressBytes = ap.IpAddress.GetAddressBytes();
                return maskBytes
                    .Select((mb, i) => (hostAddressBytes[i] & mb) == (natDeviceAddressBytes[i] & mb))
                    .All(f => f);
            }).Select(ap => ap.IpAddress).ToList();

            if (!hostAddresses.Any())
            {
                throw new ClientErrorException(ClientErrorCode.CreatingPortMappingFailed,
                    $"This host does not have IP address whose network is same as the network of the NAT device ({natDeviceAddress}).");
            }

            if (hostAddresses.Count > 1)
            {
                throw new ClientErrorException(ClientErrorCode.CreatingPortMappingFailed,
                    $"This host has multiple IP addresses ([{string.Join(", ", hostAddresses)}]) whose network is same as the network of the NAT device ({natDeviceAddress}).");
            }

            var hostAddress = hostAddresses.First();
            Logger.Log(LogLevel.Info, $"The host address whose network is same as the NAT device is {hostAddress}.");

            var mappings = (await natDevice.GetAllMappingsAsync().ConfigureAwait(false)).ToArray();
            var alreadyAvailableMappings = mappings.Where(m =>
                m.Protocol == proto && hostAddress.EqualsIpAddressSource(m.PrivateIP) &&
                privatePortCandidates.Contains((ushort)m.PrivatePort) &&
                publicPortCandidates.Contains((ushort)m.PublicPort)).ToArray();

            ushort publicPort;
            ushort privatePort;
            if (alreadyAvailableMappings.Any())
            {
                Logger.Log(LogLevel.Info,
                    $"There are already available port mappings: [{string.Join(",", alreadyAvailableMappings.Select(PortMappingToString))}].");
                var firstAvailableMapping = alreadyAvailableMappings.First();
                privatePort = (ushort)firstAvailableMapping.PrivatePort;
                publicPort = (ushort)firstAvailableMapping.PublicPort;
                Logger.Log(LogLevel.Info, $"Reuse {PortMappingToString(firstAvailableMapping)} mapping.");
            }
            else
            {
                var usedPublicPortSet =
                    new HashSet<ushort>(mappings.Where(m => m.Protocol == proto).Select(m => (ushort)m.PublicPort));
                Logger.Log(LogLevel.Info, $"Used public ports are [{string.Join(",", usedPublicPortSet)}].");

                var availablePublicPorts = publicPortCandidates.Where(p => !usedPublicPortSet.Contains(p)).ToArray();
                Logger.Log(LogLevel.Info, $"Available public ports are [{string.Join(",", availablePublicPorts)}].");
                if (!availablePublicPorts.Any())
                {
                    throw new ClientErrorException(ClientErrorCode.CreatingPortMappingFailed,
                        "There are not available public port in candidate.");
                }

                var usedPrivatePortSet = new HashSet<ushort>(mappings
                    .Where(m => m.Protocol == proto && hostAddress.EqualsIpAddressSource(m.PrivateIP))
                    .Select(m => (ushort)m.PrivatePort));
                Logger.Log(LogLevel.Info, $"Used private ports are [{string.Join(",", usedPrivatePortSet)}].");
                var availablePrivatePorts = privatePortCandidates.Where(p => !usedPrivatePortSet.Contains(p)).ToArray();
                Logger.Log(LogLevel.Info, $"Available private ports are [{string.Join(",", availablePrivatePorts)}].");
                if (!availablePrivatePorts.Any())
                {
                    throw new ClientErrorException(ClientErrorCode.CreatingPortMappingFailed,
                        "There are not available private port in candidate.");
                }

                publicPort = availablePublicPorts.First();
                privatePort = availablePrivatePorts.First();

                Logger.Log(LogLevel.Info, $"{publicPort} to {privatePort} mapping for {hostAddress} will be created.");

                await CreatePortMappingAsync(protocol, privatePort, publicPort, description).ConfigureAwait(false);
            }

            return (privatePort, publicPort);
        }

        /// <summary>
        /// Discover NAT with UPnP or PMP.
        /// </summary>
        /// <param name="timeoutMilliSeconds">Time to consider available NAT device doesn't exist in one method. Discover time is up to timeoutMilliSeconds*2 milli seconds by two methods.</param>
        /// <returns>true if NAT device is found</returns>
        public async Task<bool> DiscoverNatAsync(int timeoutMilliSeconds = 5000)
        {
            var discoverer = new NatDiscoverer();

            try
            {
                using (var cancellationTokenSource = new CancellationTokenSource(timeoutMilliSeconds))
                {
                    natDevice = await discoverer.DiscoverDeviceAsync(PortMapper.Upnp, cancellationTokenSource)
                        .ConfigureAwait(false);
                    Logger.Log(LogLevel.Info, $"A NAT device ({natDevice}) is found by UPnP");
                    IsNatDeviceAvailable = true;
                    return true;
                }
            }
            catch (NatDeviceNotFoundException)
            {
                try
                {
                    using (var cancellationTokenSource = new CancellationTokenSource(timeoutMilliSeconds))
                    {
                        Logger.Log(LogLevel.Info, "A NAT device is not found by UPnP");
                        natDevice = await discoverer.DiscoverDeviceAsync(PortMapper.Pmp, cancellationTokenSource)
                            .ConfigureAwait(false);
                        Logger.Log(LogLevel.Info, $"A NAT device ({natDevice}) is found by PMP");
                        IsNatDeviceAvailable = true;
                        return true;
                    }
                }
                catch (NatDeviceNotFoundException)
                {
                    Logger.Log(LogLevel.Info, "A NAT device is not found by PMP");
                    return false;
                }
            }
            finally
            {
                IsDiscoverNatDone = true;
            }
        }

        /// <summary>
        /// Release created port mappings if exist.
        /// </summary>
        public static void ReleaseCreatedPortMappings()
        {
            NatDiscoverer.ReleaseSessionMappings();
        }

        private readonly struct IPAddressWithPrefixLength
        {
            public IPAddressWithPrefixLength(IPAddress ipAddress, int prefixLength)
            {
                IpAddress = ipAddress;
                PrefixLength = prefixLength;
            }

            public IPAddress IpAddress { get; }
            public int PrefixLength { get; }
        }

        private NatDevice natDevice;

        private IReadOnlyList<IPAddressWithPrefixLength> GetAllIpAddressesWithMask()
        {
            var ipAddressWithMaskListList = NetworkInterface.GetAllNetworkInterfaces()
                // NetworkInterface.GetIsNetworkAvailable()の内容に基づく
                .Where(c =>
                    c.OperationalStatus == OperationalStatus.Up &&
                    c.NetworkInterfaceType != NetworkInterfaceType.Loopback &&
                    c.NetworkInterfaceType != NetworkInterfaceType.Tunnel)
                .Select(c =>
                    c.GetIPProperties().UnicastAddresses
                        .Select(u => new IPAddressWithPrefixLength(u.Address, u.PrefixLength)));

            var results = new List<IPAddressWithPrefixLength>();
            foreach (var ipAddressWithSubNetList in ipAddressWithMaskListList)
            {
                results.AddRange(ipAddressWithSubNetList);
            }

            return results;
        }
    }
}
#pragma warning restore CA1303