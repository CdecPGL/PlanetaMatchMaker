//
// Authors:
//   Lucas Ontivero lucasontivero@gmail.com 
//
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
using System.Net;

namespace Open.Nat
{
	internal class UpnpNatDeviceInfo
	{
		public UpnpNatDeviceInfo(IPAddress localAddress, Uri locationUri, string serviceControlUrl, string serviceType)
		{
			LocalAddress = localAddress;
			ServiceType = serviceType;
			HostEndPoint = new IPEndPoint(IPAddress.Parse(locationUri.Host), locationUri.Port);

			serviceControlUrl = NormalizeServiceControlUrl(serviceControlUrl);

			var builder = new UriBuilder("http", locationUri.Host, locationUri.Port);
			ServiceControlUri = new Uri(builder.Uri, serviceControlUrl);
			if (!string.Equals(ServiceControlUri.Host, locationUri.Host, StringComparison.OrdinalIgnoreCase))
			{
				throw new ArgumentException("UPnP service control URL must not change host.", nameof(serviceControlUrl));
			}
		}

		private static string NormalizeServiceControlUrl(string serviceControlUrl)
		{
			if (string.IsNullOrEmpty(serviceControlUrl) || serviceControlUrl.Trim().Length == 0)
			{
				throw new ArgumentException("UPnP service control URL is empty.", nameof(serviceControlUrl));
			}

			if (serviceControlUrl.StartsWith("//", StringComparison.Ordinal))
			{
				throw new ArgumentException("UPnP service control URL must not change host.", nameof(serviceControlUrl));
			}

			Uri absoluteUri;
			if (Uri.TryCreate(serviceControlUrl, UriKind.Absolute, out absoluteUri))
			{
				return absoluteUri.PathAndQuery;
			}

			Uri relativeUri;
			if (!Uri.TryCreate(serviceControlUrl, UriKind.Relative, out relativeUri))
			{
				throw new ArgumentException("UPnP service control URL is invalid.", nameof(serviceControlUrl));
			}

			return serviceControlUrl;
		}

		public IPEndPoint HostEndPoint { get; private set; }
		public IPAddress LocalAddress { get; private set; }
		public string ServiceType { get; private set; }
		public Uri ServiceControlUri { get; private set; }
	}
}
