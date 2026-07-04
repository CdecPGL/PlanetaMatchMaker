using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Threading.Tasks;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Open.Nat;

namespace PlanetaMatchMakerClientTest
{
    [TestClass]
    public class OpenNatReleaseTest
    {
        [TestMethod]
        public async Task ReleaseSessionMappingsAsyncDeletesOnlySessionMappings()
        {
            var device = new TestNatDevice();
            var permanentMapping = CreateMapping(Protocol.Tcp, 1000, 2000, 0);
            var manualMapping = CreateMapping(Protocol.Tcp, 1001, 2001, 60);
            var sessionMapping = CreateMapping(Protocol.Tcp, 1002, 2002, int.MaxValue);
            var forcedSessionMapping = CreateMapping(Protocol.Udp, 1003, 2003, int.MaxValue);
            forcedSessionMapping.LifetimeType = MappingLifetime.ForcedSession;

            device.AddOpenedMapping(permanentMapping);
            device.AddOpenedMapping(manualMapping);
            device.AddOpenedMapping(sessionMapping);
            device.AddOpenedMapping(forcedSessionMapping);

            await device.ReleaseSessionMappingsAsync();

            CollectionAssert.AreEquivalent(
                new[] { 2002, 2003 },
                device.DeletedMappings.Select(mapping => mapping.PublicPort).ToArray());
        }

        [TestMethod]
        public async Task ReleaseSessionMappingsAsyncWaitsForDeleteCompletion()
        {
            var device = new TestNatDevice { ExpectedDeleteCount = 1 };
            var deleteGate = new TaskCompletionSource<bool>(TaskCreationOptions.RunContinuationsAsynchronously);
            device.DeleteGate = deleteGate;
            device.AddOpenedMapping(CreateMapping(Protocol.Tcp, 1000, 2000, int.MaxValue));

            var releaseTask = device.ReleaseSessionMappingsAsync();
            await device.AllExpectedDeletesStarted;

            Assert.IsFalse(releaseTask.IsCompleted);
            deleteGate.SetResult(true);
            await releaseTask;

            Assert.IsTrue(releaseTask.IsCompleted);
        }

        [TestMethod]
        public async Task ReleaseSessionMappingsAsyncStartsDeletesInParallel()
        {
            var device = new TestNatDevice { ExpectedDeleteCount = 2 };
            var deleteGate = new TaskCompletionSource<bool>(TaskCreationOptions.RunContinuationsAsynchronously);
            device.DeleteGate = deleteGate;
            device.AddOpenedMapping(CreateMapping(Protocol.Tcp, 1000, 2000, 0));
            device.AddOpenedMapping(CreateMapping(Protocol.Tcp, 1001, 2001, int.MaxValue));
            device.AddOpenedMapping(CreateMapping(Protocol.Udp, 1002, 2002, int.MaxValue));

            var releaseTask = device.ReleaseSessionMappingsAsync();
            var completedTask = await Task.WhenAny(device.AllExpectedDeletesStarted, Task.Delay(1000));

            Assert.AreSame(device.AllExpectedDeletesStarted, completedTask);
            Assert.IsFalse(releaseTask.IsCompleted);
            CollectionAssert.AreEquivalent(
                new[] { 2001, 2002 },
                device.DeletedMappings.Select(mapping => mapping.PublicPort).ToArray());

            deleteGate.SetResult(true);
            await releaseTask;
        }

        private static Mapping CreateMapping(Protocol protocol, int privatePort, int publicPort, int lifetime)
        {
            return new Mapping(
                protocol,
                IPAddress.Parse("192.168.1.2"),
                privatePort,
                publicPort,
                lifetime,
                "PlanetaMatchMakerClientTest");
        }

        private sealed class TestNatDevice : NatDevice
        {
            private readonly object syncObject = new object();
            private readonly TaskCompletionSource<bool> allExpectedDeletesStarted =
                new TaskCompletionSource<bool>(TaskCreationOptions.RunContinuationsAsynchronously);

            public override IPEndPoint HostEndPoint { get; } =
                new IPEndPoint(IPAddress.Parse("192.168.1.1"), 1900);

            public override IPAddress LocalAddress { get; } = IPAddress.Parse("192.168.1.2");

            public List<Mapping> DeletedMappings { get; } = new List<Mapping>();

            public int ExpectedDeleteCount { get; set; }

            public TaskCompletionSource<bool> DeleteGate { get; set; }

            public Task AllExpectedDeletesStarted => allExpectedDeletesStarted.Task;

            public void AddOpenedMapping(Mapping mapping)
            {
                RegisterMapping(mapping);
            }

            public override Task CreatePortMapAsync(Mapping mapping)
            {
                RegisterMapping(mapping);
                return Task.CompletedTask;
            }

            public override async Task DeletePortMapAsync(Mapping mapping)
            {
                lock (syncObject)
                {
                    DeletedMappings.Add(mapping);
                    if (ExpectedDeleteCount == 0 || DeletedMappings.Count == ExpectedDeleteCount)
                    {
                        allExpectedDeletesStarted.TrySetResult(true);
                    }
                }

                if (DeleteGate != null)
                {
                    await DeleteGate.Task.ConfigureAwait(false);
                }

                UnregisterMapping(mapping);
            }

            public override Task<IEnumerable<Mapping>> GetAllMappingsAsync()
            {
                return Task.FromResult(Enumerable.Empty<Mapping>());
            }

            public override Task<IPAddress> GetExternalIPAsync()
            {
                return Task.FromResult(IPAddress.Parse("203.0.113.1"));
            }

            public override Task<Mapping> GetSpecificMappingAsync(Protocol protocol, int port)
            {
                return Task.FromResult<Mapping>(null);
            }
        }
    }
}
