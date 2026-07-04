//
// Authors:
//   Ben Motmans <ben.motmans@gmail.com>
//   Lucas Ontivero lucasontivero@gmail.com
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
using System.IO;
using System.Linq;
using System.Text;
using System.Net;
using System.Diagnostics;
using System.Net.Sockets;
using System.Threading;
using System.Xml;

namespace Open.Nat
{
	internal class UpnpSearcher : Searcher
	{
		private readonly IIPAddressesProvider _ipprovider;
		private readonly IDictionary<Uri, NatDevice> _devices;
		private readonly Dictionary<IPAddress, DateTime> _lastFetched;
		private static readonly string[] ServiceTypes = new[]{
			"WANIPConnection:2", 
			"WANPPPConnection:2",
			"WANIPConnection:1", 
			"WANPPPConnection:1" 
		};

		internal UpnpSearcher(IIPAddressesProvider ipprovider)
		{
			_ipprovider = ipprovider;
			UdpClients = CreateUdpClients();
			_devices = new Dictionary<Uri, NatDevice>();
			_lastFetched = new Dictionary<IPAddress, DateTime>();
		}

		private List<UdpClient> CreateUdpClients()
		{
			var clients = new List<UdpClient>();
			try
			{
				var ips = _ipprovider.UnicastAddresses();

				foreach (var ipAddress in ips)
				{
					try
					{
						clients.Add(new UdpClient(new IPEndPoint(ipAddress, 0)));
					}
					catch (Exception)
					{
						continue; // Move on to the next address.
					}
				}
			}
			catch (Exception)
			{
				clients.Add(new UdpClient(0));
			}
			return clients;
		}

		protected override void Discover(UdpClient client, CancellationToken cancelationToken)
		{
			// for testing use: 
			//    <code>var ip = IPAddress.Broadcast;</code>
			Discover(client, WellKnownConstants.IPv4MulticastAddress, cancelationToken);
			if (Socket.OSSupportsIPv6)
			{
				Discover(client, WellKnownConstants.IPv6LinkLocalMulticastAddress, cancelationToken);
				Discover(client, WellKnownConstants.IPv6LinkSiteMulticastAddress, cancelationToken);
			}
		}

		private void Discover(UdpClient client, IPAddress address, CancellationToken cancelationToken)
		{
			if (!IsValidClient(client.Client, address)) return;
			
			NextSearch = DateTime.UtcNow.AddSeconds(1);
			var searchEndpoint = new IPEndPoint(address, 1900);

			foreach (var serviceType in ServiceTypes)
			{
				var datax = DiscoverDeviceMessage.Encode(serviceType, address);
				var data = Encoding.ASCII.GetBytes(datax);

				// UDP is unreliable, so send 3 requests at a time (per Upnp spec, sec 1.1.2)
				// Yes, however it works perfectly well with just 1 request.
				for (var i = 0; i < 3; i++)
				{
					if (cancelationToken.IsCancellationRequested) return;
					client.Send(data, data.Length, searchEndpoint);
				}
			}
		}

		private bool IsValidClient(Socket socket, IPAddress address)
		{
			var endpoint = (IPEndPoint) socket.LocalEndPoint;
			if (socket.AddressFamily != address.AddressFamily) return false;

			switch (socket.AddressFamily)
			{
				case AddressFamily.InterNetwork:
					socket.SetSocketOption(SocketOptionLevel.IP, SocketOptionName.MulticastInterface, endpoint.Address.GetAddressBytes());
					return true;
				case AddressFamily.InterNetworkV6:
					if (endpoint.Address.IsIPv6LinkLocal && !Equals(address, WellKnownConstants.IPv6LinkLocalMulticastAddress))
						return false;
					if (!endpoint.Address.IsIPv6LinkLocal && !Equals(address, WellKnownConstants.IPv6LinkSiteMulticastAddress))
						return false;

					socket.SetSocketOption(SocketOptionLevel.IPv6, SocketOptionName.MulticastInterface, BitConverter.GetBytes((int)endpoint.Address.ScopeId));
					return true;
			}
			return false;
		}

		public override NatDevice AnalyseReceivedResponse(IPAddress localAddress, byte[] response, IPEndPoint endpoint)
		{
			// Convert it to a string for easy parsing
			string dataString = null;

			// No matter what, this method should never throw an exception. If something goes wrong
			// we should still be in a position to handle the next reply correctly.
			try
			{
				dataString = Encoding.UTF8.GetString(response);
				var message = new DiscoveryResponseMessage(dataString);
				var serviceType = message["ST"];
				string validatedServiceType;

				if (!TryGetValidControllerService(serviceType, out validatedServiceType))
				{
					NatDiscoverer.TraceSource.LogWarn("Invalid controller service. Ignoring.");

					return null;
				}
				serviceType = validatedServiceType;
				NatDiscoverer.TraceSource.LogInfo("UPnP Response: Router advertised a '{0}' service!!!", serviceType);

				var locationUri = GetValidatedLocationUri(message, endpoint);
				if (locationUri == null)
				{
					return null;
				}
				NatDiscoverer.TraceSource.LogInfo("Found device at: {0}", locationUri.ToString());

				if (_devices.ContainsKey(locationUri))
				{
					NatDiscoverer.TraceSource.LogInfo("Already found - Ignored");
					_devices[locationUri].Touch();
					return null;
				}

				// If we send 3 requests at a time, ensure we only fetch the services list once
				// even if three responses are received
				if (_lastFetched.ContainsKey(endpoint.Address))
				{
					var last = _lastFetched[endpoint.Address];
					if ((DateTime.Now - last) < TimeSpan.FromSeconds(20))
						return null;
				}
				_lastFetched[endpoint.Address] = DateTime.Now;

				NatDiscoverer.TraceSource.LogInfo("{0}:{1}: Fetching service list", locationUri.Host, locationUri.Port );

				var deviceInfo = BuildUpnpNatDeviceInfo(localAddress, locationUri);

				UpnpNatDevice device;
				lock (_devices)
				{
					device = new UpnpNatDevice(deviceInfo);
					if (!_devices.ContainsKey(locationUri))
					{
						_devices.Add(locationUri, device);
					}
				}
				return device;
			}
			catch (Exception ex)
			{
				NatDiscoverer.TraceSource.LogError("Unhandled exception when trying to decode a device's response. ");
				NatDiscoverer.TraceSource.LogError("Report the issue in https://github.com/lontivero/Open.Nat/issues");
				NatDiscoverer.TraceSource.LogError("Also copy and paste the following info:");
				NatDiscoverer.TraceSource.LogError("-- beging ---------------------------------");
				NatDiscoverer.TraceSource.LogError(ex.Message);
				NatDiscoverer.TraceSource.LogError("Data string:");
				NatDiscoverer.TraceSource.LogError(dataString ?? "No data available");
				NatDiscoverer.TraceSource.LogError("-- end ------------------------------------");
			}
			return null;
		}

		private static bool TryGetValidControllerService(string serviceType, out string serviceUrn)
		{
			serviceUrn = null;
			if (serviceType == null)
			{
				return false;
			}

			var normalizedServiceType = serviceType.Trim();
			var services = from serviceName in ServiceTypes
						   let candidateServiceUrn = "urn:schemas-upnp-org:service:" + serviceName
						   where string.Equals(normalizedServiceType, candidateServiceUrn, StringComparison.OrdinalIgnoreCase)
						   select candidateServiceUrn;

			serviceUrn = services.FirstOrDefault();
			return serviceUrn != null;
		}

		private static Uri GetValidatedLocationUri(DiscoveryResponseMessage message, IPEndPoint endpoint)
		{
			string location;
			if (!message.TryGetValue("Location", out location) && !message.TryGetValue("AL", out location))
			{
				NatDiscoverer.TraceSource.LogWarn("UPnP response did not include a location. Ignoring.");
				return null;
			}

			Uri locationUri;
			if (!Uri.TryCreate(location, UriKind.Absolute, out locationUri))
			{
				NatDiscoverer.TraceSource.LogWarn("UPnP location is not an absolute URI. Ignoring: {0}", location);
				return null;
			}

			if (!string.Equals(locationUri.Scheme, Uri.UriSchemeHttp, StringComparison.OrdinalIgnoreCase))
			{
				NatDiscoverer.TraceSource.LogWarn("UPnP location scheme is not supported. Ignoring: {0}", locationUri.Scheme);
				return null;
			}

			IPAddress locationAddress;
			if (!TryGetCanonicalLocationHostAddress(location, out locationAddress))
			{
				NatDiscoverer.TraceSource.LogWarn("UPnP location host is not an IP address. Ignoring: {0}", locationUri.Host);
				return null;
			}

			if (!AreSameDeviceAddress(locationAddress, endpoint.Address))
			{
				NatDiscoverer.TraceSource.LogWarn(
					"UPnP location host does not match the SSDP response address. Ignoring: {0} != {1}",
					locationAddress, endpoint.Address);
				return null;
			}

			return locationUri;
		}

		private static bool TryGetCanonicalLocationHostAddress(string location, out IPAddress address)
		{
			address = null;

			if (location == null)
			{
				return false;
			}

			var trimmedLocation = location.Trim();
			var schemeSeparatorIndex = trimmedLocation.IndexOf("://", StringComparison.Ordinal);
			if (schemeSeparatorIndex < 0)
			{
				return false;
			}

			var authorityStartIndex = schemeSeparatorIndex + 3;
			var authorityEndIndex = trimmedLocation.IndexOfAny(new[] {'/', '?', '#'}, authorityStartIndex);
			if (authorityEndIndex < 0)
			{
				authorityEndIndex = trimmedLocation.Length;
			}

			var authority = trimmedLocation.Substring(authorityStartIndex, authorityEndIndex - authorityStartIndex);
			if (authority.Length == 0 || authority.IndexOf('@') >= 0)
			{
				return false;
			}

			string host;
			if (authority[0] == '[')
			{
				var hostEndIndex = authority.IndexOf(']');
				if (hostEndIndex < 0)
				{
					return false;
				}

				host = authority.Substring(1, hostEndIndex - 1);
				var remainder = authority.Substring(hostEndIndex + 1);
				if (!IsValidPortSuffix(remainder) || host.IndexOf('%') >= 0)
				{
					return false;
				}

				if (!IPAddress.TryParse(host, out address) || address.AddressFamily != AddressFamily.InterNetworkV6 ||
					address.IsIPv4MappedToIPv6)
				{
					return false;
				}

				return true;
			}

			var colonIndex = authority.IndexOf(':');
			if (colonIndex >= 0)
			{
				if (authority.IndexOf(':', colonIndex + 1) >= 0)
				{
					return false;
				}

				host = authority.Substring(0, colonIndex);
				if (!IsValidPortSuffix(authority.Substring(colonIndex)))
				{
					return false;
				}
			}
			else
			{
				host = authority;
			}

			return TryParseCanonicalIPv4Host(host, out address);
		}

		private static bool IsValidPortSuffix(string portSuffix)
		{
			if (portSuffix.Length == 0)
			{
				return true;
			}

			if (portSuffix[0] != ':' || portSuffix.Length == 1)
			{
				return false;
			}

			var portText = portSuffix.Substring(1);
			if (portText.Any(c => c < '0' || c > '9'))
			{
				return false;
			}

			int port;
			return int.TryParse(portText, NumberStyles.None, CultureInfo.InvariantCulture, out port) &&
				port >= IPEndPoint.MinPort &&
				port <= IPEndPoint.MaxPort;
		}

		private static bool TryParseCanonicalIPv4Host(string host, out IPAddress address)
		{
			address = null;
			var parts = host.Split('.');
			if (parts.Length != 4)
			{
				return false;
			}

			var bytes = new byte[4];
			for (var index = 0; index < parts.Length; ++index)
			{
				var part = parts[index];
				if (part.Length == 0 || (part.Length > 1 && part[0] == '0') || part.Any(c => c < '0' || c > '9'))
				{
					return false;
				}

				int value;
				if (!int.TryParse(part, NumberStyles.None, CultureInfo.InvariantCulture, out value) ||
					value < byte.MinValue ||
					value > byte.MaxValue)
				{
					return false;
				}

				bytes[index] = (byte)value;
			}

			address = new IPAddress(bytes);
			return string.Equals(address.ToString(), host, StringComparison.Ordinal);
		}

		private static bool AreSameDeviceAddress(IPAddress left, IPAddress right)
		{
			left = NormalizeAddress(left);
			right = NormalizeAddress(right);

			return left.AddressFamily == right.AddressFamily && left.GetAddressBytes().SequenceEqual(right.GetAddressBytes());
		}

		private static IPAddress NormalizeAddress(IPAddress address)
		{
			return address.IsIPv4MappedToIPv6 ? address.MapToIPv4() : address;
		}

		private UpnpNatDeviceInfo BuildUpnpNatDeviceInfo(IPAddress localAddress, Uri location)
		{
			NatDiscoverer.TraceSource.LogInfo("Found device at: {0}", location.ToString());

			var hostEndPoint = new IPEndPoint(IPAddress.Parse(location.Host), location.Port);

			WebResponse response = null;
			try
			{
#if NET35
				var request = (HttpWebRequest)WebRequest.Create(location);
#else
				var request = WebRequest.CreateHttp(location);
#endif
				request.AllowAutoRedirect = false;
				request.Headers.Add("ACCEPT-LANGUAGE", "en");
				request.Method = "GET";

				response = request.GetResponse();

				var httpresponse = response as HttpWebResponse;

				if (httpresponse != null && httpresponse.StatusCode != HttpStatusCode.OK)
				{
					var message = string.Format("Couldn't get services list: {0} {1}", httpresponse.StatusCode, httpresponse.StatusDescription);
					throw new Exception(message);
				}

				var xmldoc = ReadXmlResponse(response);

				NatDiscoverer.TraceSource.LogInfo("{0}: Parsed services list", hostEndPoint);

				var ns = new XmlNamespaceManager(xmldoc.NameTable);
				ns.AddNamespace("ns", "urn:schemas-upnp-org:device-1-0");
				var services = xmldoc.SelectNodes("//ns:service", ns);

				foreach (XmlNode service in services)
				{
					var serviceType = service.GetXmlElementText("serviceType");
					string validatedServiceType;
					if (!TryGetValidControllerService(serviceType, out validatedServiceType)) continue;
					serviceType = validatedServiceType;

					NatDiscoverer.TraceSource.LogInfo("{0}: Found service: {1}", hostEndPoint, serviceType);

					var serviceControlUrl = service.GetXmlElementText("controlURL");
					NatDiscoverer.TraceSource.LogInfo("{0}: Found upnp service at: {1}", hostEndPoint, serviceControlUrl);

					NatDiscoverer.TraceSource.LogInfo("{0}: Handshake Complete", hostEndPoint);
					return new UpnpNatDeviceInfo(localAddress, location, serviceControlUrl, serviceType);
				}

				throw new Exception("No valid control service was found in the service descriptor document");
			}
			catch (WebException ex)
			{
				// Just drop the connection, FIXME: Should i retry?
				NatDiscoverer.TraceSource.LogError("{0}: Device denied the connection attempt: {1}", hostEndPoint, ex);
				var inner = ex.InnerException as SocketException;
				if (inner != null)
				{
					NatDiscoverer.TraceSource.LogError("{0}: ErrorCode:{1}", hostEndPoint, inner.ErrorCode);
					NatDiscoverer.TraceSource.LogError("Go to http://msdn.microsoft.com/en-us/library/system.net.sockets.socketerror.aspx");
					NatDiscoverer.TraceSource.LogError("Usually this happens. Try resetting the device and try again. If you are in a VPN, disconnect and try again.");
				}
				throw;
			}
			finally
			{
				if (response != null)
					response.Close();
			}
		}

		private static XmlDocument ReadXmlResponse(WebResponse response)
		{
			return StreamExtensions.GetXmlDocument(response.ReadXmlResponseBody());
		}
	}
}
