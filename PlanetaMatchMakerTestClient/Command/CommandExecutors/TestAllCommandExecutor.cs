using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Threading;
using System.Threading.Tasks;

namespace PlanetaGameLabo.MatchMaker
{
    internal sealed class TestAllCommandExecutor : TestCommandExecutorBase<TestAllCommandOptions>
    {
        public TestAllCommandExecutor(StreamWriter outputStream) : base(outputStream)
        {
        }

        public override string Explanation => "Test all commands.";
        public override Command command => Command.TestAll;

        protected override void InitializeTest(
            List<(string name, Func<MatchMakerClient, TestAllCommandOptions, CancellationToken, Task>)> tests)
        {
            InitializeParameters();
            tests.Add((nameof(DiscoverNatDevice), DiscoverNatDevice));
            tests.Add((nameof(CreatePortMapping), CreatePortMapping));
            tests.Add((nameof(ListPortMappingsAndCheckCreated), ListPortMappingsAndCheckCreated));
            tests.Add((nameof(RemovePortMappings), RemovePortMappings));
            tests.Add((nameof(ListPortMappingsAndCheckRemoved), ListPortMappingsAndCheckRemoved));
            tests.Add((nameof(Connect), Connect));
            tests.Add((nameof(ListRoomGroupList), ListRoomGroupList));
            tests.Add((nameof(CreateRoom), CreateRoom));
            tests.Add((nameof(ListRoomAndCheckRoomCreated), ListRoomAndCheckRoomCreated));
            tests.Add((nameof(RemoveRoom), RemoveRoom));
            tests.Add((nameof(ListRoomAndCheckRoomRemoved), ListRoomAndCheckRoomRemoved));
            tests.Add((nameof(CreateRoomWithCreatingPortMapping), CreateRoomWithCreatingPortMapping));
            tests.Add((nameof(ListRoomAndCheckRoomCreated), ListRoomAndCheckRoomCreated));
            tests.Add((nameof(RemoveRoom), RemoveRoom));
            tests.Add((nameof(CreateRoomWithCreatingPortMapping), CreateRoomWithCreatingPortMapping));
            tests.Add((nameof(SecondClientConnect), SecondClientConnect));
            tests.Add((nameof(SecondClientListRoomAndCheckRoomCreatedByHost),
                SecondClientListRoomAndCheckRoomCreatedByHost));
            tests.Add((nameof(SecondClientJoinRoom), SecondClientJoinRoom));
        }

        protected override void FinalizeTest(MatchMakerClient sharedClient, TestAllCommandOptions options)
        {
            if (sharedClient.Connected)
            {
                sharedClient.Close();
            }

            if (secondClient != null && secondClient.Connected)
            {
                secondClient.Close();
            }
        }

        private void InitializeParameters()
        {
            playerName = Guid.NewGuid().ToString("N").Substring(0, 10);
            OutputStream.WriteLine($"playerName: {playerName}");
        }

        private async Task DiscoverNatDevice(MatchMakerClient sharedClient, TestAllCommandOptions options,
            CancellationToken cancellationToken)
        {
            if (!await sharedClient.PortMappingCreator.DiscoverNatAsync(options.DiscoverTimeoutMilliSeconds))
            {
                throw new TestFailedException(true, "NAT device is not found.");
            }
        }

        private async Task CreatePortMapping(MatchMakerClient sharedClient, TestAllCommandOptions options,
            CancellationToken cancellationToken)
        {
            try
            {
                var portCandidates = options.GameHostPortCandidates.ToList();
                portCandidates.Insert(0, options.GameHostDefaultPort);
                (lastCreatedPrivatePort, lastCreatedPublicPort) =
                    await sharedClient.PortMappingCreator.CreatePortMappingFromCandidatesAsync(options.GameHostProtocol,
                        portCandidates, portCandidates);
            }
            catch (ClientErrorException e)
            {
                throw new TestFailedException(true, e.Message);
            }
            catch (InvalidOperationException e)
            {
                throw new TestFailedException(true, e.Message);
            }
        }

        private async Task ListPortMappingsAndCheckCreated(MatchMakerClient sharedClient, TestAllCommandOptions options,
            CancellationToken cancellationToken)
        {
            try
            {
                var portMappings = await sharedClient.PortMappingCreator.GetAllPortMappingsAsync();
                if (!portMappings.Any(m =>
                    m.PrivatePort == lastCreatedPrivatePort && m.PublicPort == lastCreatedPublicPort))
                {
                    throw new TestFailedException(true, "Created port mapping is not found in list of port mappings.");
                }
            }
            catch (ClientErrorException e)
            {
                throw new TestFailedException(true, e.Message);
            }
            catch (InvalidOperationException e)
            {
                throw new TestFailedException(true, e.Message);
            }
        }

        private async Task RemovePortMappings(MatchMakerClient sharedClient, TestAllCommandOptions options,
            CancellationToken cancellationToken)
        {
            NatPortMappingCreator.ReleaseCreatedPortMappings();
            // We wait 1 second because it is possible that removing port mappings is not finished just after the operation.
            OutputStream.WriteLine("Wait 1 second.");
            await Task.Delay(1000, cancellationToken);
        }

        private async Task ListPortMappingsAndCheckRemoved(MatchMakerClient sharedClient, TestAllCommandOptions options,
            CancellationToken cancellationToken)
        {
            try
            {
                var portMappings = await sharedClient.PortMappingCreator.GetAllPortMappingsAsync();
                if (portMappings.Any(m =>
                    m.PrivatePort == lastCreatedPrivatePort && m.PublicPort == lastCreatedPublicPort))
                {
                    throw new TestFailedException(true,
                        "Created port mapping is not removed in list of port mappings. The cause is sometimes removing port mappings is not finished.");
                }
            }
            catch (ClientErrorException e)
            {
                throw new TestFailedException(true, e.Message);
            }
            catch (InvalidOperationException e)
            {
                throw new TestFailedException(true, e.Message);
            }
        }

        private async Task Connect(MatchMakerClient sharedClient, TestAllCommandOptions options,
            CancellationToken cancellationToken)
        {
            try
            {
                var playerFullName =
                    await sharedClient.ConnectAsync(options.ServerAddress, options.ServerPort, playerName);
                if (playerFullName.Name != playerName)
                {
                    throw new TestFailedException(false,
                        $"Returned player name ({playerFullName.Name}) doesn't match passed one ({playerName}).");
                }
            }
            catch (ClientErrorException e)
            {
                throw new TestFailedException(false, e.Message);
            }
            catch (ArgumentException e)
            {
                throw new TestFailedException(false, e.Message);
            }
        }

        private async Task ListRoomGroupList(MatchMakerClient sharedClient, TestAllCommandOptions options,
            CancellationToken cancellationToken)
        {
            try
            {
                var roomGroupList = await sharedClient.GetRoomGroupListAsync();
                if (!roomGroupList.Any())
                {
                    throw new TestFailedException(true, "No room group is listed in ListRoomGroupRequest.");
                }
            }
            catch (ClientErrorException e)
            {
                throw new TestFailedException(false, e.Message);
            }
            catch (ArgumentException e)
            {
                throw new TestFailedException(false, e.Message);
            }
        }

        private async Task CreateRoom(MatchMakerClient sharedClient, TestAllCommandOptions options,
            CancellationToken cancellationToken)
        {
            try
            {
                await sharedClient.CreateRoomAsync(0, 8, options.GameHostDefaultPort);
                lastHostedRoomId = sharedClient.HostingRoomId;
            }
            catch (ClientErrorException e)
            {
                throw new TestFailedException(true, e.Message);
            }
            catch (ArgumentException e)
            {
                throw new TestFailedException(true, e.Message);
            }
        }

        private async Task ListRoomAndCheckRoomCreated(MatchMakerClient sharedClient, TestAllCommandOptions options,
            CancellationToken cancellationToken)
        {
            try
            {
                var (_, _, roomInfoList) =
                    await sharedClient.GetRoomListAsync(0, 0, 100, RoomDataSortKind.NameAscending);
                if (roomInfoList.All(r => r.RoomId != lastHostedRoomId && r.HostPlayerFullName == sharedClient.PlayerFullName))
                {
                    throw new TestFailedException(true, "Created room is not listed in ListRoomRequest.");
                }
            }
            catch (ClientErrorException e)
            {
                throw new TestFailedException(true, e.Message);
            }
        }

        private async Task RemoveRoom(MatchMakerClient sharedClient, TestAllCommandOptions options,
            CancellationToken cancellationToken)
        {
            try
            {
                await sharedClient.RemoveHostingRoomAsync();
            }
            catch (ClientErrorException e)
            {
                throw new TestFailedException(true, e.Message);
            }
        }

        private async Task ListRoomAndCheckRoomRemoved(MatchMakerClient sharedClient, TestAllCommandOptions options,
            CancellationToken cancellationToken)
        {
            try
            {
                var (_, _, roomInfoList) =
                    await sharedClient.GetRoomListAsync(0, 0, 100, RoomDataSortKind.NameAscending);
                if (roomInfoList.Any(r => r.RoomId == lastHostedRoomId))
                {
                    throw new TestFailedException(true, "Created room is not removed in ListRoomRequest.");
                }
            }
            catch (ClientErrorException e)
            {
                throw new TestFailedException(true, e.Message);
            }
        }

        private async Task CreateRoomWithCreatingPortMapping(MatchMakerClient sharedClient,
            TestAllCommandOptions options, CancellationToken cancellationToken)
        {
            try
            {
                if (!sharedClient.PortMappingCreator.IsDiscoverNatDone)
                {
                    OutputStream.WriteLine("Execute NAT discovering because it is not done.");
                    await sharedClient.PortMappingCreator.DiscoverNatAsync();
                }

                await sharedClient.CreateRoomWithCreatingPortMappingAsync(0, 8, options.GameHostProtocol,
                    options.GameHostPortCandidates, options.GameHostDefaultPort, options.DiscoverTimeoutMilliSeconds);
                lastHostedRoomId = sharedClient.HostingRoomId;
            }
            catch (ClientErrorException e)
            {
                throw new TestFailedException(true, e.Message);
            }
            catch (ArgumentException e)
            {
                throw new TestFailedException(true, e.Message);
            }
        }

        private async Task SecondClientConnect(MatchMakerClient sharedClient, TestAllCommandOptions options,
            CancellationToken cancellationToken)
        {
            try
            {
                secondClient = new MatchMakerClient(sharedClient.TimeoutMilliSeconds, sharedClient.Logger);
                await secondClient.ConnectAsync(options.ServerAddress, options.ServerPort, playerName);
            }
            catch (ClientErrorException e)
            {
                throw new TestFailedException(false, e.Message);
            }
            catch (ArgumentException e)
            {
                throw new TestFailedException(false, e.Message);
            }
        }

        private async Task SecondClientListRoomAndCheckRoomCreatedByHost(MatchMakerClient sharedClient,
            TestAllCommandOptions options, CancellationToken cancellationToken)
        {
            try
            {
                var (_, _, roomInfoList) =
                    await secondClient.GetRoomListAsync(0, 0, 100, RoomDataSortKind.NameAscending);
                if (roomInfoList.All(r => r.RoomId != lastHostedRoomId))
                {
                    throw new TestFailedException(true, "Room created by host is not listed in ListRoomRequest.");
                }
            }
            catch (ClientErrorException e)
            {
                throw new TestFailedException(true, e.Message);
            }
        }

        private async Task SecondClientJoinRoom(MatchMakerClient sharedClient, TestAllCommandOptions options,
            CancellationToken cancellationToken)
        {
            try
            {
                await secondClient.JoinRoomAsync(0, lastHostedRoomId);
            }
            catch (ClientErrorException e)
            {
                throw new TestFailedException(true, e.Message);
            }
        }

        private string playerName;
        private uint lastHostedRoomId;
        private MatchMakerClient secondClient;
        private ushort lastCreatedPrivatePort;
        private ushort lastCreatedPublicPort;
    }

    internal class TestAllCommandOptions : TestCommandOptions
    {
        [CommandLine.Value(0, MetaName = "ServerAddress", Required = true, HelpText = "An address of the server.")]
        public string ServerAddress { get; set; }

        [CommandLine.Value(1, MetaName = "ServerPort", Required = true, HelpText = "A port of the server.")]
        public ushort ServerPort { get; set; }

        [CommandLine.Option('p', "game_host_default_port", Default = (ushort)52000, Required = false,
            HelpText = "A port used to host game.")]
        public ushort GameHostDefaultPort { get; set; }

        [CommandLine.Option('o', "game_host_protocol", Default = TransportProtocol.Tcp, Required = false,
            HelpText = "A protocol used to host game.")]
        public TransportProtocol GameHostProtocol { get; set; }

        [CommandLine.Option('t', "discover_timeout_milli_seconds", Default = 5000, Required = false,
            HelpText = "Timeout milli seconds to discover NAT.")]
        public int DiscoverTimeoutMilliSeconds { get; set; }

        [CommandLine.Option('c', "game_host_port_candidates", Default = new ushort[] {53000, 54000}, Required = false,
            Separator = ',', HelpText = "Candidates of port to map.")]
        public IEnumerable<ushort> GameHostPortCandidates { get; set; }
    }
}