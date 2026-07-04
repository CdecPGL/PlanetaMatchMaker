//
// Authors:
//   Alan McGovern alan.mcgovern@gmail.com
//   Ben Motmans <ben.motmans@gmail.com>
//   Lucas Ontivero lucasontivero@gmail.com
//
// Copyright (C) 2006 Alan McGovern
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
using System.Linq;
using System.Net;
using System.Threading.Tasks;

namespace Open.Nat
{
	/// <summary>
	/// Represents a NAT device and provides access to the operation set that allows
	/// open (forward) ports, close ports and get the externa (visible) IP address.
	/// </summary>
	public abstract class NatDevice
	{
		/// <summary>
		/// A local endpoint of NAT device.
		/// </summary>
		public abstract IPEndPoint HostEndPoint { get; }

		/// <summary>
		/// A local IP address of client.
		/// </summary>
		public abstract IPAddress LocalAddress { get; }

		private readonly HashSet<Mapping> _openedMapping = new HashSet<Mapping>();
		private readonly object _openedMappingLock = new object();
		protected DateTime LastSeen { get; private set; }

		internal void Touch()
		{
			LastSeen = DateTime.Now;
		}

		/// <summary>
		/// Creates the port map asynchronous.
		/// </summary>
		/// <param name="mapping">The <see cref="Mapping">Mapping</see> entry.</param>
		/// <example>
		/// device.CreatePortMapAsync(new Mapping(Protocol.Tcp, 1700, 1600));
		/// </example>
		/// <exception cref="MappingException">MappingException</exception>
		public abstract Task CreatePortMapAsync(Mapping mapping);

		/// <summary>
		/// Deletes a mapped port asynchronous.
		/// </summary>
		/// <param name="mapping">The <see cref="Mapping">Mapping</see> entry.</param>
		/// <example>
		/// device.DeletePortMapAsync(new Mapping(Protocol.Tcp, 1700, 1600));
		/// </example>
		/// <exception cref="MappingException">MappingException-class</exception>
		public abstract Task DeletePortMapAsync(Mapping mapping);

		/// <summary>
		/// Gets all mappings asynchronous.
		/// </summary>
		/// <returns>
		/// The list of all forwarded ports
		/// </returns>
		/// <example>
		/// var mappings = await device.GetAllMappingsAsync();
		/// foreach(var mapping in mappings)
		/// {
		///	 Console.WriteLine(mapping)
		/// }
		/// </example>
		/// <exception cref="MappingException">MappingException</exception>
		public abstract Task<IEnumerable<Mapping>> GetAllMappingsAsync();

		/// <summary>
		/// Gets the external (visible) IP address asynchronous. This is the NAT device IP address
		/// </summary>
		/// <returns>
		/// The public IP addrees
		/// </returns>
		/// <example>
		/// Console.WriteLine("My public IP is: {0}", await device.GetExternalIPAsync());
		/// </example>
		/// <exception cref="MappingException">MappingException</exception>
		public abstract Task<IPAddress> GetExternalIPAsync();

		/// <summary>
		/// Gets the specified mapping asynchronous.
		/// </summary>
		/// <param name="protocol">The protocol.</param>
		/// <param name="port">The port.</param>
		/// <returns>
		/// The matching mapping
		/// </returns>
		public abstract Task<Mapping> GetSpecificMappingAsync(Protocol protocol, int port);

		protected void RegisterMapping(Mapping mapping)
		{
			lock (_openedMappingLock)
			{
				_openedMapping.Remove(mapping);
				_openedMapping.Add(mapping);
			}
		}

		protected void UnregisterMapping(Mapping mapping)
		{
			lock (_openedMappingLock)
			{
				_openedMapping.RemoveWhere(x => x.Equals(mapping));
			}
		}


		internal async Task ReleaseMappingAsync(IEnumerable<Mapping> mappings)
		{
			var maparr = mappings.ToArray();
			var mapCount = maparr.Length;
			NatDiscoverer.TraceSource.LogInfo("{0} ports to close", mapCount);
			var tasks = maparr.Select(async mapping =>
			{
				try
				{
					await DeletePortMapAsync(mapping).ConfigureAwait(false);
					NatDiscoverer.TraceSource.LogInfo(mapping + " port successfully closed");
				}
				catch (Exception)
				{
					NatDiscoverer.TraceSource.LogError(mapping + " port couldn't be close");
				}
			}).ToArray();
			await Task.WhenAll(tasks).ConfigureAwait(false);
		}

		internal async Task ReleaseAllAsync()
		{
			Mapping[] mappings;
			lock (_openedMappingLock)
			{
				mappings = _openedMapping.ToArray();
			}

			await ReleaseMappingAsync(mappings).ConfigureAwait(false);
		}

		internal async Task ReleaseSessionMappingsAsync()
		{
			Mapping[] mappings;
			lock (_openedMappingLock)
			{
				mappings = (from m in _openedMapping
							where m.LifetimeType == MappingLifetime.Session ||
								  m.LifetimeType == MappingLifetime.ForcedSession
							select m).ToArray();
			}

			await ReleaseMappingAsync(mappings).ConfigureAwait(false);
		}

		internal async Task RenewMappings()
		{
			Mapping[] mappings;
			lock (_openedMappingLock)
			{
				mappings = _openedMapping.Where(x => x.ShoundRenew()).ToArray();
			}

			foreach (var mapping in mappings)
			{
				var m = mapping;
				await RenewMapping(m);
			}
		}

		private async Task RenewMapping(Mapping mapping)
		{
			var renewMapping = new Mapping(mapping);
			try
			{
				renewMapping.Expiration = DateTime.UtcNow.AddSeconds(mapping.Lifetime);

				NatDiscoverer.TraceSource.LogInfo("Renewing mapping {0}", renewMapping);
				await CreatePortMapAsync(renewMapping);
				NatDiscoverer.TraceSource.LogInfo("Next renew scheduled at: {0}",
												  renewMapping.Expiration.ToLocalTime().TimeOfDay);
			}
			catch (Exception)
			{
				NatDiscoverer.TraceSource.LogWarn("Renew {0} failed", mapping);
			}
		}
	}
}
