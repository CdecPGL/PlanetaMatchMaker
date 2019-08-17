using System;
using System.Net;
using PlanetaGameLabo.Serializer;

namespace PlanetaGameLabo.MatchMaker
{
    [Serializable]
    public struct ClientAddress
    {
        [FixedLength(16)] public byte[] IpAddress;

        public ushort PortNumber;

        public IPAddress IpAddressInstance => new IPAddress(IpAddress);

        public bool IsIpv4 => IpAddressInstance.IsIPv4MappedToIPv6;

        public override string ToString()
        {
            var ipAddressText = IsIpv4
                ? IpAddressInstance.MapToIPv4().ToString()
                : IpAddressInstance.ToString();

            return $"{nameof(ClientAddress)}({ipAddressText}:{PortNumber})";
        }
    }
}