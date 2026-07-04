//
// Authors:
//   Ben Motmans <ben.motmans@gmail.com>
//   Lucas Ontivero lucas.ontivero@gmail.com
//
// Copyright (C) 2007 Ben Motmans
// Copyright (C) 2014 Lucas Ontivero
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
// 
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//

using System;
using System.Collections.Generic;
using System.Globalization;
using System.Net;
using System.Net.Sockets;
using System.Threading.Tasks;

namespace Open.Nat
{
	internal sealed class PmpNatDevice : NatDevice
	{
		public override IPEndPoint HostEndPoint
		{
			get { return _hostEndPoint; }
		}

		public override IPAddress LocalAddress
		{
			get { return _localAddress; }
		}

		private readonly IPEndPoint _hostEndPoint;
		private readonly IPAddress _localAddress;
		private readonly IPAddress _publicAddress;
		private static readonly TimeSpan PortMapTimeout = TimeSpan.FromSeconds(4);

		internal PmpNatDevice(IPAddress localAddress, IPAddress gatewayAddress, IPAddress publicAddress)
		{
			_hostEndPoint = new IPEndPoint(gatewayAddress, PmpConstants.ServerPort);
			_localAddress = localAddress;
			_publicAddress = publicAddress;
		}

#if NET35
		public override Task CreatePortMapAsync(Mapping mapping)
		{
			return InternalCreatePortMapAsync(mapping, true)
				.TimeoutAfter(TimeSpan.FromSeconds(4))
				.ContinueWith(t => RegisterMapping(mapping));
		}
#else
		public override async Task CreatePortMapAsync(Mapping mapping)
		{
			await InternalCreatePortMapAsync(mapping, true)
				.TimeoutAfter(PortMapTimeout)
				.ConfigureAwait(false);
			RegisterMapping(mapping);
		}
#endif

#if NET35
		public override Task DeletePortMapAsync(Mapping mapping)
		{
			return InternalCreatePortMapAsync(mapping, false)
				.TimeoutAfter(TimeSpan.FromSeconds(4))
				.ContinueWith(t => UnregisterMapping(mapping));
		}
#else
		public override async Task DeletePortMapAsync(Mapping mapping)
		{
			await InternalCreatePortMapAsync(mapping, false)
				.TimeoutAfter(PortMapTimeout)
				.ConfigureAwait(false);
			UnregisterMapping(mapping);
		}
#endif

		public override Task<IEnumerable<Mapping>> GetAllMappingsAsync()
		{
			throw new NotSupportedException();
		}

		public override Task<IPAddress> GetExternalIPAsync()
		{
#if NET35
			return Task.Factory.StartNew(() => _publicAddress)
#else
			return Task.Run(() => _publicAddress)
#endif
				.TimeoutAfter(TimeSpan.FromSeconds(4));
		}

		public override Task<Mapping> GetSpecificMappingAsync(Protocol protocol, int port)
		{
			throw new NotSupportedException("NAT-PMP does not specify a way to get a specific port map");
		}

#if NET35
		private Task<Mapping> InternalCreatePortMapAsync(Mapping mapping, bool create)
		{
			return Task.Factory.StartNew(() =>
			{
				try
				{
					byte[] buffer = BuildPortMapRequestPackage(mapping, create);
					int delay = PmpConstants.RetryDelay;
					DateTime deadline = DateTime.UtcNow.Add(PortMapTimeout);

					using (var udpClient = new UdpClient())
					{
						for (int attempt = 0; attempt < PmpConstants.RetryAttempts; attempt++)
						{
							int receiveTimeout = GetReceiveTimeout(delay, deadline);
							if (receiveTimeout <= 0)
								break;

							udpClient.Send(buffer, buffer.Length, HostEndPoint);

							if (TryReceivePortMapResponse(udpClient, mapping, create, receiveTimeout))
								return mapping;

							delay *= 2;
						}
					}
				}
				catch (MappingException)
				{
					throw;
				}
				catch (Exception e)
				{
					throw CreateMappingException(mapping, create, e);
				}

				throw CreateMappingException(mapping, create, null);
			});
		}
#else
		private Task<Mapping> InternalCreatePortMapAsync(Mapping mapping, bool create)
		{
			return Task.Run(() =>
			{
				try
				{
					byte[] buffer = BuildPortMapRequestPackage(mapping, create);
					int delay = PmpConstants.RetryDelay;
					DateTime deadline = DateTime.UtcNow.Add(PortMapTimeout);

					using (var udpClient = new UdpClient())
					{
						for (int attempt = 0; attempt < PmpConstants.RetryAttempts; attempt++)
						{
							int receiveTimeout = GetReceiveTimeout(delay, deadline);
							if (receiveTimeout <= 0)
								break;

							udpClient.Send(buffer, buffer.Length, HostEndPoint);

							if (TryReceivePortMapResponse(udpClient, mapping, create, receiveTimeout))
								return mapping;

							delay *= 2;
						}
					}
				}
				catch (MappingException)
				{
					throw;
				}
				catch (Exception e)
				{
					throw CreateMappingException(mapping, create, e);
				}

				throw CreateMappingException(mapping, create, null);
			});
		}
#endif

		private static byte[] BuildPortMapRequestPackage(Mapping mapping, bool create)
		{
			var buffer = new byte[12];
			buffer[0] = PmpConstants.Version;
			buffer[1] = GetOperationCode(mapping.Protocol);
			WriteUInt16(buffer, 4, mapping.PrivatePort);
			WriteUInt16(buffer, 6, create ? mapping.PublicPort : 0);
			WriteUInt32(buffer, 8, mapping.Lifetime);
			return buffer;
		}

		private static byte GetOperationCode(Protocol protocol)
		{
			return protocol == Protocol.Tcp ? PmpConstants.OperationCodeTcp : PmpConstants.OperationCodeUdp;
		}

		private static int GetReceiveTimeout(int delay, DateTime deadline)
		{
			var remaining = (int)(deadline - DateTime.UtcNow).TotalMilliseconds;
			if (remaining <= 0)
				return 0;

			return Math.Min(delay, remaining);
		}

		private bool TryReceivePortMapResponse(UdpClient udpClient, Mapping mapping, bool create, int timeout)
		{
			udpClient.Client.ReceiveTimeout = timeout;

			try
			{
				var endPoint = new IPEndPoint(IPAddress.Any, 0);
				byte[] data = udpClient.Receive(ref endPoint);
				return TryApplyPortMapResponse(data, endPoint, mapping, create);
			}
			catch (SocketException e)
			{
				if (e.SocketErrorCode == SocketError.TimedOut || e.SocketErrorCode == SocketError.WouldBlock)
					return false;

				throw;
			}
		}

		private bool TryApplyPortMapResponse(byte[] data, IPEndPoint endpoint, Mapping mapping, bool create)
		{
			if (!IsExpectedResponseEndpoint(endpoint)
				|| data.Length != 16
				|| data[0] != PmpConstants.Version)
				return false;

			byte expectedOperationCode = (byte)(PmpConstants.ServerNoop + GetOperationCode(mapping.Protocol));
			if (data[1] != expectedOperationCode)
				return false;

			short resultCode = (short)ReadUInt16(data, 2);
			if (resultCode != PmpConstants.ResultCodeSuccess)
				throw new MappingException(resultCode, GetPmpErrorText(resultCode));

			int privatePort = ReadUInt16(data, 8);
			int publicPort = ReadUInt16(data, 10);
			uint lifetime = ReadUInt32(data, 12);

			if (privatePort != mapping.PrivatePort)
				return false;

			if (!create)
				return publicPort == 0 && lifetime == 0;

			if (lifetime == 0)
				return false;

			double lifetimeSeconds = lifetime > int.MaxValue ? int.MaxValue : lifetime;
			mapping.PublicPort = publicPort;
			mapping.Expiration = DateTime.UtcNow.AddSeconds(lifetimeSeconds);
			return true;
		}

		private bool IsExpectedResponseEndpoint(IPEndPoint endpoint)
		{
			return endpoint.Address.Equals(HostEndPoint.Address) && endpoint.Port == HostEndPoint.Port;
		}

		private static void WriteUInt16(byte[] buffer, int offset, int value)
		{
			buffer[offset] = (byte)(value >> 8);
			buffer[offset + 1] = (byte)value;
		}

		private static void WriteUInt32(byte[] buffer, int offset, int value)
		{
			buffer[offset] = (byte)(value >> 24);
			buffer[offset + 1] = (byte)(value >> 16);
			buffer[offset + 2] = (byte)(value >> 8);
			buffer[offset + 3] = (byte)value;
		}

		private static int ReadUInt16(byte[] buffer, int offset)
		{
			return (buffer[offset] << 8) | buffer[offset + 1];
		}

		private static uint ReadUInt32(byte[] buffer, int offset)
		{
			return ((uint)buffer[offset] << 24)
				| ((uint)buffer[offset + 1] << 16)
				| ((uint)buffer[offset + 2] << 8)
				| buffer[offset + 3];
		}

		private static string GetPmpErrorText(int resultCode)
		{
			var errors = new[]
			{
				"Success",
				"Unsupported Version",
				"Not Authorized/Refused (e.g. box supports mapping, but user has turned feature off)",
				"Network Failure (e.g. NAT box itself has not obtained a DHCP lease)",
				"Out of resources (NAT box cannot create any more mappings at this time)",
				"Unsupported opcode"
			};

			if (resultCode >= 0 && resultCode < errors.Length)
				return errors[resultCode];

			return "Unknown NAT-PMP error";
		}

		private static MappingException CreateMappingException(Mapping mapping, bool create, Exception innerException)
		{
			string type = create ? "create" : "delete";
			string message = String.Format(CultureInfo.InvariantCulture,
										   "Failed to {0} portmap (protocol={1}, private port={2})",
										   type,
										   mapping.Protocol,
										   mapping.PrivatePort);
			NatDiscoverer.TraceSource.LogError(message);
			return innerException == null
				? new MappingException(message)
				: new MappingException(message, innerException);
		}

		public override string ToString()
		{
			return String.Format(CultureInfo.InvariantCulture,
								 "Local Address: {0}\nPublic IP: {1}\nLast Seen: {2}",
								 HostEndPoint.Address, _publicAddress, LastSeen);
		}
	}
}
