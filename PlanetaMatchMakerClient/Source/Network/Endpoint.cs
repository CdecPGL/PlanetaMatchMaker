using System;
using System.Net;
using CdecPGL.MinimalSerializer;

namespace PlanetaGameLabo.MatchMaker
{
    [Serializable]
    public struct EndPoint
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

            return $"{nameof(EndPoint)}({ipAddressText}:{PortNumber})";
        }

        public static explicit operator IPEndPoint(EndPoint value)
        {
            return new IPEndPoint(value.IpAddressInstance, value.PortNumber);
        }
    }
}