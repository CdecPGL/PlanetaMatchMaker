using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Threading;
using System.Threading.Tasks;
using Open.Nat;

namespace PlanetaGameLabo.MatchMaker
{
    /// <summary>
    /// A class to create port mapping to NAT device which is compatible with UPnP or PMP.
    /// </summary>
    public sealed class NatPortMappingCreator
    {
        /// <summary>
        /// true if NAT there are available NAT device.
        /// </summary>
        public bool IsNatDeviceAvailable { get; private set; }

        /// <summary>
        /// true if DiscoverNat is executed at least once.
        /// </summary>
        public bool IsDiscoverNatDone { get; private set; }

        /// <summary>
        /// Create port mapping.
        /// </summary>
        /// <param name="protocol"></param>
        /// <param name="privatePort"></param>
        /// <param name="publicPort"></param>
        /// <param name="description"></param>
        /// <exception cref="InvalidOperationException">DiscoverNat is not executed or NAT device is not available</exception>
        /// <exception cref="ClientErrorException">Failed to create port mapping</exception>
        /// <returns></returns>
        public async Task CreatePortMapping(TransportProtocol protocol, ushort privatePort, ushort publicPort,
            string description)
        {
            if (!IsDiscoverNatDone)
            {
                throw new InvalidOperationException("DiscoverNat should be executed before this method.");
            }

            if (!IsNatDeviceAvailable)
            {
                throw new InvalidOperationException("NAT device is not available.");
            }

            try
            {
                await natDevice.CreatePortMapAsync(new Mapping(
                    protocol == TransportProtocol.Tcp ? Protocol.Tcp : Protocol.Udp, privatePort, publicPort,
                    description));
            }
            catch (MappingException e)
            {
                throw new ClientErrorException(ClientErrorCode.CreatingPortMappingFailed, e.Message);
            }
        }

        /// <summary>
        /// Create port mapping by select available port pair from candidate.
        /// </summary>
        /// <param name="protocol"></param>
        /// <param name="candidatePrivatePorts"></param>
        /// <param name="candidatePublicPorts"></param>
        /// <param name="description"></param>
        /// <exception cref="InvalidOperationException">DiscoverNat is not executed or NAT device is not available</exception>
        /// <exception cref="ClientErrorException">Failed to create port mapping. There may be not available port pair in the candidates</exception>
        /// <returns></returns>
        public async Task<(ushort privatePort, ushort publicPort)> CreatePortMappingFromCandidate(
            TransportProtocol protocol,
            ICollection<ushort> candidatePrivatePorts,
            ICollection<ushort> candidatePublicPorts, string description)
        {
            if (!IsDiscoverNatDone)
            {
                throw new InvalidOperationException("DiscoverNat should be executed before this method.");
            }

            if (!IsNatDeviceAvailable)
            {
                throw new InvalidOperationException("NAT device is not available.");
            }

            var proto = protocol == TransportProtocol.Tcp ? Protocol.Tcp : Protocol.Udp;
            var hostname = Dns.GetHostName();
            var myAddresses = await Dns.GetHostAddressesAsync(hostname);
            var mappings = (await natDevice.GetAllMappingsAsync()).ToArray();

            var alreadyAvailableMappings = mappings.Where(m =>
                m.Protocol == proto && myAddresses.Contains(m.PrivateIP) &&
                candidatePrivatePorts.Contains((ushort)m.PrivatePort) &&
                candidatePublicPorts.Contains((ushort)m.PublicPort)).ToArray();

            ushort publicPort;
            ushort privatePort;
            if (alreadyAvailableMappings.Any())
            {
                privatePort = (ushort)alreadyAvailableMappings.First().PrivatePort;
                publicPort = (ushort)alreadyAvailableMappings.First().PublicPort;
            }
            else
            {
                var usedPublicPortSet =
                    new HashSet<ushort>(mappings.Where(m => m.Protocol == proto).Select(m => (ushort)m.PublicPort));
                var availablePublicPorts = candidatePublicPorts.Where(p => !usedPublicPortSet.Contains(p)).ToArray();
                if (!availablePublicPorts.Any())
                {
                    throw new ClientErrorException(ClientErrorCode.CreatingPortMappingFailed,
                        "There are not available public port in candidate.");
                }

                var usedPrivatePortSet = new HashSet<ushort>(mappings
                    .Where(m => m.Protocol == proto && myAddresses.Contains(m.PrivateIP))
                    .Select(m => (ushort)m.PrivatePort));
                var availablePrivatePorts = candidatePrivatePorts.Where(p => !usedPrivatePortSet.Contains(p)).ToArray();
                if (!availablePrivatePorts.Any())
                {
                    throw new ClientErrorException(ClientErrorCode.CreatingPortMappingFailed,
                        "There are not available private port in candidate.");
                }

                publicPort = availablePublicPorts.First();
                privatePort = availablePrivatePorts.First();
            }

            await CreatePortMapping(protocol, privatePort, publicPort, description);
            return (privatePort, publicPort);
        }

        /// <summary>
        /// Discover NAT with UPnP or PMP.
        /// </summary>
        /// <param name="timeoutMilliSeconds">Time to consider available NAT device doesn't exist in one method. Discover time is up to timeoutMilliSeconds*2 milli seconds by two methods.</param>
        /// <returns></returns>
        public async Task DiscoverNat(int timeoutMilliSeconds)
        {
            var discoverer = new NatDiscoverer();
            var cts = new CancellationTokenSource(timeoutMilliSeconds);

            try
            {
                natDevice = await discoverer.DiscoverDeviceAsync(PortMapper.Upnp, cts);
                IsNatDeviceAvailable = true;
            }
            catch (NatDeviceNotFoundException)
            {
                try
                {
                    natDevice = await discoverer.DiscoverDeviceAsync(PortMapper.Pmp, cts);
                    IsNatDeviceAvailable = true;
                }
                catch (NatDeviceNotFoundException)
                {
                }
            }
            finally
            {
                IsDiscoverNatDone = true;
            }
        }

        private NatDevice natDevice;
    }
}