using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Net.Sockets;
using System.Reflection;
using System.Threading;
using System.Threading.Tasks;
using CdecPGL.MinimalSerializer;

#pragma warning disable CA1303
namespace PlanetaGameLabo.MatchMaker
{
    /// <summary>
    /// Match maker client.
    /// Call NatPortMappingCreator.ReleaseCreatedPortMappings method manually to ensure to release port mappings created in the application.
    /// <see cref="NatPortMappingCreator.ReleaseCreatedPortMappings"/>
    /// </summary>
    public sealed class MatchMakerClient : IDisposable
    {
        /// <summary>
        /// true if connected to the server.
        /// </summary>
        public bool Connected => tcpClient != null && tcpClient.Connected;

        /// <summary>
        /// true if this client hosting a room.
        /// </summary>
        public bool IsHostingRoom { get; private set; }

        /// <summary>
        /// Hosting room id. This is valid when hosting room.
        /// </summary>
        public uint HostingRoomId { get; private set; }

        /// <summary>
        /// Hosting room id. This is valid when hosting room.
        /// </summary>
        public byte HostingRoomGroupIndex { get; private set; }

        /// <summary>
        /// An instance of class to create port mapping to NAT.
        /// </summary>
        public NatPortMappingCreator PortMappingCreator { get; }

        /// <summary>
        /// Logger used in this instance.
        /// </summary>
        public ILogger Logger { get; }

        /// <summary>
        /// コンストラクタ。
        /// </summary>
        /// <param name="timeoutMilliSeconds">Timeout milli seconds for send and receive. Timeout of connect is not effected.</param>
        /// <param name="logger"></param>
        public MatchMakerClient(int timeoutMilliSeconds = 10000, ILogger logger = null)
        {
            if (logger == null)
            {
                logger = StreamLogger.CreateStandardOutputLogger();
            }

            this.timeoutMilliSeconds = timeoutMilliSeconds;
            Logger = logger;
            PortMappingCreator = new NatPortMappingCreator(logger);
        }

        /// <summary>
        /// Connect to matching server.
        /// </summary>
        /// <param name="serverAddress"></param>
        /// <param name="serverPort"></param>
        /// <exception cref="ClientErrorException"></exception>
        /// <exception cref="ArgumentException"></exception>
        /// <returns></returns>
        public async Task ConnectAsync(string serverAddress, ushort serverPort)
        {
            await semaphore.WaitAsync().ConfigureAwait(false);
            try
            {
                if (string.IsNullOrWhiteSpace(serverAddress))
                {
                    throw new ArgumentException("Value cannot be null or whitespace.", nameof(serverAddress));
                }

                if (Connected)
                {
                    throw new ClientErrorException(ClientErrorCode.AlreadyConnected);
                }

                // Establish TCP connection
                try
                {
                    tcpClient = CreateTcpClient();
                    await tcpClient.ConnectAsync(serverAddress, serverPort).ConfigureAwait(false);
                    Logger.Log(LogLevel.Info, $"Connect to {serverAddress}:{serverPort} successfully.");
                }
                catch (SocketException e)
                {
                    throw new ClientErrorException(ClientErrorCode.FailedToConnect, e.Message);
                }
                catch (ObjectDisposedException e)
                {
                    throw new ClientErrorException(ClientErrorCode.FailedToConnect, e.Message);
                }

                // Authentication Request
                var requestBody = new AuthenticationRequestMessage {Version = ClientConstants.ApiVersion};
                await SendRequestAsync(requestBody).ConfigureAwait(false);
                Logger.Log(LogLevel.Info,
                    $"Send AuthenticationRequest. ({nameof(ClientConstants.ApiVersion)}: {ClientConstants.ApiVersion})");
                var replyBody = await ReceiveReplyAsync<AuthenticationReplyMessage>().ConfigureAwait(false);
                Logger.Log(LogLevel.Info,
                    $"Receive AuthenticationReply. ({nameof(replyBody.SessionKey)}: {replyBody.SessionKey}, {nameof(replyBody.Version)}: {replyBody.Version})");
                sessionKey = replyBody.SessionKey;
            }
            finally
            {
                semaphore.Release();
            }
        }

        /// <summary>
        /// Close the connection.
        /// </summary>
        /// <exception cref="ClientErrorException">Not connected</exception>
        public void Close()
        {
            if (!Connected)
            {
                throw new ClientErrorException(ClientErrorCode.NotConnected);
            }

            tcpClient.GetStream().Close();
            tcpClient.Close();
            tcpClient.Dispose();
            tcpClient = null;
            Logger.Log(LogLevel.Info, $"Close connection to the server.");
            OnConnectionClosed();
        }

        /// <summary>
        /// Get a room group list from the server.
        /// </summary>
        /// <exception cref="ClientErrorException"></exception>
        /// <exception cref="ClientInternalErrorException"></exception>
        /// <returns></returns>
        public async Task<RoomGroupResult[]> GetRoomGroupListAsync()
        {
            await semaphore.WaitAsync().ConfigureAwait(false);
            try
            {
                if (!Connected)
                {
                    throw new ClientErrorException(ClientErrorCode.NotConnected);
                }

                var requestBody = new ListRoomGroupRequestMessage();
                await SendRequestAsync(requestBody).ConfigureAwait(false);
                Logger.Log(LogLevel.Info, $"Send ListRoomGroupRequest.");

                var replyBody = await ReceiveReplyAsync<ListRoomGroupReplyMessage>().ConfigureAwait(false);
                Logger.Log(LogLevel.Info,
                    $"Receive ListRoomGroupReply. ({nameof(replyBody.RoomGroupCount)}: {replyBody.RoomGroupCount})");
                return replyBody.RoomGroupInfoList.Take(replyBody.RoomGroupCount)
                    .Select(info => new RoomGroupResult(info))
                    .ToArray();
            }
            catch (ClientInternalErrorException)
            {
                if (tcpClient.Connected)
                {
                    Close();
                }

                throw;
            }
            finally
            {
                semaphore.Release();
            }
        }

        /// <summary>
        /// Create and host new room to the server.
        /// </summary>
        /// <param name="roomGroupIndex"></param>
        /// <param name="roomName"></param>
        /// <param name="maxPlayerCount"></param>
        /// <param name="portNumber">The port number which is used for accept TCP connection of game</param>
        /// <param name="isPublic"></param>
        /// <param name="password"></param>
        /// <exception cref="ClientErrorException"></exception>
        /// <exception cref="ClientInternalErrorException"></exception>
        /// <exception cref="ArgumentException"></exception>
        /// <returns></returns>
        public async Task CreateRoomAsync(byte roomGroupIndex, string roomName, byte maxPlayerCount, ushort portNumber,
            bool isPublic = true, string password = "")
        {
            await semaphore.WaitAsync().ConfigureAwait(false);
            try
            {
                if (roomName == null)
                {
                    throw new ArgumentNullException(nameof(roomName));
                }

                if (password == null)
                {
                    throw new ArgumentNullException(nameof(password));
                }

                if (!Connected)
                {
                    throw new ClientErrorException(ClientErrorCode.NotConnected);
                }

                if (IsHostingRoom)
                {
                    throw new ClientErrorException(ClientErrorCode.AlreadyHostingRoom,
                        "The client can host only one room.");
                }

                await CreateRoomCoreAsync(portNumber, roomGroupIndex, roomName, maxPlayerCount, isPublic, password)
                    .ConfigureAwait(false);
            }
            catch (ClientInternalErrorException)
            {
                if (tcpClient.Connected)
                {
                    Close();
                }

                throw;
            }
            finally
            {
                semaphore.Release();
            }
        }

        /// <summary>
        /// Create and host new room to the server with creating port mapping to NAT.
        /// This method doesn't create port mapping if first connection test is succeed.
        /// If first connection test is failed, this method try to create port mapping.
        /// The port pair to map is selected from candidates which consists of default port and port candidates from parameter.
        /// The ports which is being used by this computer with indicated protocol is removed from candidates.
        /// Refer NatPortMappingCreator.CreatePortMappingFromCandidates method if you want to know selection rule of port from candidates.
        /// If second connection test after port mapping is created is failed, this method throws error.
        /// </summary>
        /// <param name="roomGroupIndex"></param>
        /// <param name="roomName"></param>
        /// <param name="maxPlayerCount"></param>
        /// <param name="discoverNatTimeoutMilliSeconds"></param>
        /// <param name="protocol">The protocol which is used for accept TCP connection of game</param>
        /// <param name="portNumberCandidates">The candidates of port number which is used for accept TCP connection of game</param>
        /// <param name="defaultPortNumber">The port number which is tried to use for accept TCP connection of game first</param>
        /// <param name="isPublic"></param>
        /// <param name="password"></param>
        /// <exception cref="ClientErrorException"></exception>
        /// <exception cref="ClientInternalErrorException"></exception>
        /// <exception cref="ArgumentException"></exception>
        /// <returns></returns>
        public async Task<CreateRoomWithCreatingPortMappingResult> CreateRoomWithCreatingPortMappingAsync(
            byte roomGroupIndex, string roomName,
            byte maxPlayerCount, TransportProtocol protocol, IEnumerable<ushort> portNumberCandidates,
            ushort defaultPortNumber, int discoverNatTimeoutMilliSeconds = 5000, bool isPublic = true,
            string password = "")
        {
            await semaphore.WaitAsync().ConfigureAwait(false);
            try
            {
                if (roomName == null)
                {
                    throw new ArgumentNullException(nameof(roomName));
                }

                if (password == null)
                {
                    throw new ArgumentNullException(nameof(password));
                }

                if (!Connected)
                {
                    throw new ClientErrorException(ClientErrorCode.NotConnected);
                }

                if (IsHostingRoom)
                {
                    throw new ClientErrorException(ClientErrorCode.AlreadyHostingRoom,
                        "The client can host only one room.");
                }

                if (!PortMappingCreator.IsNatDeviceAvailable || !PortMappingCreator.IsDiscoverNatDone)
                {
                    Logger.Log(LogLevel.Info, "Execute discovering NAT device because it is not done.");
                    await PortMappingCreator.DiscoverNat(discoverNatTimeoutMilliSeconds).ConfigureAwait(false);
                }

                if (!PortMappingCreator.IsNatDeviceAvailable)
                {
                    throw new ClientErrorException(ClientErrorCode.CreatingPortMappingFailed,
                        "Failed to discover NAT device.");
                }

                var isDefaultPortUsed = true;
                ushort usedPrivatePortFromCandidates = 0;
                ushort usedPublicPortFromCandidates = 0;

                Logger.Log(LogLevel.Info, $"Execute first connection test (Default port: {defaultPortNumber}).");
                var connectionTestSucceed = false;
                try
                {
                    connectionTestSucceed =
                        await ConnectionTestCoreAsync(protocol, defaultPortNumber).ConfigureAwait(false);
                }
                // Consider port already used error as connection test failure
                catch (InvalidOperationException)
                {
                }

                var portNumber = defaultPortNumber;
                if (!connectionTestSucceed)
                {
                    Logger.Log(LogLevel.Info, "Try to create port mapping because connection test is failed.");
                    // Create port candidates
                    var portNumberCandidateArray = portNumberCandidates.ToList();
                    Logger.Log(LogLevel.Debug, $"Port candidates: [{string.Join(",", portNumberCandidateArray)}]");
                    if (!portNumberCandidateArray.Contains(defaultPortNumber))
                    {
                        portNumberCandidateArray.Add(defaultPortNumber);
                    }

                    // Remove used ports in this machine
                    var privatePortCandidates =
                        NetworkHelper.FilterPortsByAvailability(protocol, portNumberCandidateArray).ToArray();
                    Logger.Log(LogLevel.Debug, $"Private port candidates: [{string.Join(",", privatePortCandidates)}]");
                    var publicPortCandidates = portNumberCandidateArray;
                    Logger.Log(LogLevel.Debug, $"Public port candidates: [{string.Join(",", publicPortCandidates)}]");

                    isDefaultPortUsed = false;
                    (usedPrivatePortFromCandidates, usedPublicPortFromCandidates) = await PortMappingCreator
                        .CreatePortMappingFromCandidates(protocol, privatePortCandidates, publicPortCandidates)
                        .ConfigureAwait(false);
                    portNumber = usedPublicPortFromCandidates;
                    Logger.Log(LogLevel.Info,
                        $"Port mapping is created in NAT. (privatePortNumber: {usedPrivatePortFromCandidates}, publicPortNumber: {usedPublicPortFromCandidates})");

                    Logger.Log(LogLevel.Info, "Execute second connection test.");

                    connectionTestSucceed = await ConnectionTestCoreAsync(protocol, portNumber).ConfigureAwait(false);
                    if (!connectionTestSucceed)
                    {
                        throw new ClientErrorException(ClientErrorCode.NotReachable);
                    }

                    Logger.Log(LogLevel.Info, "Second connection test is succeeded. Start to create room.");
                }

                await CreateRoomCoreAsync(portNumber, roomGroupIndex, roomName, maxPlayerCount, isPublic, password)
                    .ConfigureAwait(false);

                return new CreateRoomWithCreatingPortMappingResult(isDefaultPortUsed, usedPrivatePortFromCandidates,
                    usedPublicPortFromCandidates);
            }
            catch (ClientInternalErrorException)
            {
                if (tcpClient.Connected)
                {
                    Close();
                }

                throw;
            }
            finally
            {
                semaphore.Release();
            }
        }

        /// <summary>
        /// Get a room list from the server.
        /// </summary>
        /// <param name="roomGroupIndex"></param>
        /// <param name="startIndex"></param>
        /// <param name="count"></param>
        /// <param name="sortKind"></param>
        /// <param name="searchTargetFlags"></param>
        /// <param name="searchName"></param>
        /// <exception cref="ClientErrorException"></exception>
        /// <exception cref="ClientInternalErrorException"></exception>
        /// <returns></returns>
        public async Task<(byte totalRoomCount, byte matchedRoomCount, RoomResult[] roomInfoList)> GetRoomListAsync(
            byte roomGroupIndex, byte startIndex,
            byte count, RoomDataSortKind sortKind, RoomSearchTargetFlag searchTargetFlags = RoomSearchTargetFlag.All,
            string searchName = "")
        {
            await semaphore.WaitAsync().ConfigureAwait(false);
            try
            {
                if (searchName == null)
                {
                    throw new ArgumentNullException(nameof(searchName));
                }

                var searchNameMaxLength = typeof(ListRoomRequestMessage)
                    .GetField(nameof(ListRoomRequestMessage.SearchName))
                    .GetCustomAttribute<FixedLengthAttribute>().Length;
                if (searchName.Length > searchNameMaxLength)
                {
                    throw new ArgumentNullException(
                        $"The length of {nameof(searchName)} must be less than {searchNameMaxLength}.");
                }

                if (!Connected)
                {
                    throw new ClientErrorException(ClientErrorCode.NotConnected);
                }

                var requestBody = new ListRoomRequestMessage
                {
                    GroupIndex = roomGroupIndex,
                    StartIndex = startIndex,
                    Count = count,
                    SortKind = sortKind,
                    SearchTargetFlags = searchTargetFlags,
                    SearchName = searchName
                };
                await SendRequestAsync(requestBody).ConfigureAwait(false);
                Logger.Log(LogLevel.Info,
                    $"Send ListRoomRequest. ({nameof(requestBody.GroupIndex)}: {requestBody.GroupIndex}, {nameof(requestBody.StartIndex)}: {requestBody.StartIndex}, {nameof(requestBody.Count)}: {requestBody.Count}, {nameof(requestBody.SortKind)}: {requestBody.SortKind}, {nameof(requestBody.SearchTargetFlags)}: {requestBody.SearchTargetFlags}, {nameof(requestBody.SearchName)}: {requestBody.SearchName})");

                var replyBody = await ReceiveReplyAsync<ListRoomReplyMessage>().ConfigureAwait(false);
                Logger.Log(LogLevel.Info,
                    $"Receive ListRoomReply. ({nameof(replyBody.TotalRoomCount)}: {replyBody.TotalRoomCount}, {nameof(replyBody.MatchedRoomCount)}: {replyBody.MatchedRoomCount}, {nameof(replyBody.ReplyRoomCount)}: {replyBody.ReplyRoomCount})");
                var result = new RoomResult[replyBody.ReplyRoomCount];

                // Set results of reply to result list
                var currentIndex = requestBody.StartIndex;

                void SetResult(in ListRoomReplyMessage reply)
                {
                    for (var i = 0; i < RoomConstants.ListRoomReplyRoomInfoCount; ++i)
                    {
                        if (currentIndex >= replyBody.ReplyRoomCount)
                        {
                            return;
                        }

                        result[currentIndex++] = new RoomResult(reply.RoomInfoList[i]);
                    }
                }

                SetResult(replyBody);

                var separateCount = (replyBody.ReplyRoomCount - 1) / RoomConstants.ListRoomReplyRoomInfoCount + 1;

                for (var i = 1; i < separateCount; ++i)
                {
                    replyBody = await ReceiveReplyAsync<ListRoomReplyMessage>().ConfigureAwait(false);
                    Logger.Log(LogLevel.Info, $"Receive additional ListRoomReply ({i + 1}/{separateCount}).");
                    SetResult(replyBody);
                }

                return (replyBody.TotalRoomCount, replyBody.MatchedRoomCount, result);
            }
            catch (ClientInternalErrorException)
            {
                if (tcpClient.Connected)
                {
                    Close();
                }

                throw;
            }
            finally
            {
                semaphore.Release();
            }
        }

        /// <summary>
        /// Join to a room on the server.
        /// </summary>
        /// <param name="roomGroupIndex"></param>
        /// <param name="roomId"></param>
        /// <param name="password"></param>
        /// <exception cref="ClientErrorException"></exception>
        /// <exception cref="ClientInternalErrorException"></exception>
        /// <returns>Game host endpoint</returns>
        public async Task<IPEndPoint> JoinRoomAsync(byte roomGroupIndex, uint roomId, string password = "")
        {
            await semaphore.WaitAsync().ConfigureAwait(false);
            try
            {
                if (!Connected)
                {
                    throw new ClientErrorException(ClientErrorCode.NotConnected);
                }

                if (IsHostingRoom)
                {
                    throw new ClientErrorException(ClientErrorCode.AlreadyHostingRoom,
                        "The client hosting a room can't join the room.");
                }

                var requestBody = new JoinRoomRequestMessage
                {
                    GroupIndex = roomGroupIndex, RoomId = roomId, Password = password
                };
                await SendRequestAsync(requestBody).ConfigureAwait(false);
                Logger.Log(LogLevel.Info,
                    $"Send JoinRoomRequest. ({nameof(requestBody.GroupIndex)}: {requestBody.GroupIndex}, {nameof(requestBody.RoomId)}: {requestBody.RoomId}, {nameof(requestBody.Password)}: {requestBody.Password})");

                var replyBody = await ReceiveReplyAsync<JoinRoomReplyMessage>().ConfigureAwait(false);
                Logger.Log(LogLevel.Info,
                    $"Receive JoinRoomReply. ({nameof(replyBody.GameHostEndPoint)}: {replyBody.GameHostEndPoint})");

                Close();
                return (IPEndPoint)replyBody.GameHostEndPoint;
            }
            catch (ClientInternalErrorException)
            {
                if (tcpClient.Connected)
                {
                    Close();
                }

                throw;
            }
            finally
            {
                semaphore.Release();
            }
        }

        /// <summary>
        /// Update hosting room status on the server.
        /// This method doesn't notice error in the server because the API this method uses doesn't reply.
        /// </summary>
        /// <param name="roomStatus"></param>
        /// <param name="updateCurrentPlayerCount"></param>
        /// <param name="currentPlayerCount"></param>
        /// <exception cref="ClientErrorException"></exception>
        /// <exception cref="ClientInternalErrorException"></exception>
        /// <returns></returns>
        public async Task UpdateHostingRoomStatusAsync(RoomStatus roomStatus, bool updateCurrentPlayerCount = false,
            byte currentPlayerCount = 0)
        {
            await semaphore.WaitAsync().ConfigureAwait(false);
            try
            {
                if (!Connected)
                {
                    throw new ClientErrorException(ClientErrorCode.NotConnected);
                }

                if (!IsHostingRoom)
                {
                    throw new ClientErrorException(ClientErrorCode.NotHostingRoom);
                }

                var requestBody = new UpdateRoomStatusNoticeMessage
                {
                    GroupIndex = HostingRoomGroupIndex,
                    RoomId = HostingRoomId,
                    Status = roomStatus,
                    IsCurrentPlayerCountChanged = updateCurrentPlayerCount,
                    CurrentPlayerCount = currentPlayerCount
                };
                await SendRequestAsync(requestBody).ConfigureAwait(false);
                Logger.Log(LogLevel.Info,
                    $"Send UpdateRoomStatusNotice. ({nameof(requestBody.GroupIndex)}: {requestBody.GroupIndex}, {nameof(requestBody.RoomId)}: {requestBody.RoomId}, {nameof(requestBody.Status)}: {requestBody.Status}, {nameof(requestBody.IsCurrentPlayerCountChanged)}: {requestBody.IsCurrentPlayerCountChanged}, {nameof(requestBody.CurrentPlayerCount)}: {requestBody.CurrentPlayerCount})");

                if (roomStatus == RoomStatus.Remove)
                {
                    IsHostingRoom = false;
                }
            }
            catch (ClientInternalErrorException)
            {
                if (tcpClient.Connected)
                {
                    Close();
                }

                throw;
            }
            finally
            {
                semaphore.Release();
            }
        }

        /// <summary>
        /// Remove hosting room from the server.
        /// </summary>
        /// <exception cref="ClientErrorException"></exception>
        /// <exception cref="ClientInternalErrorException"></exception>
        /// <returns></returns>
        public async Task RemoveHostingRoomAsync()
        {
            await UpdateHostingRoomStatusAsync(RoomStatus.Remove).ConfigureAwait(false);
        }

        /// <summary>
        /// Check if other machine can connect to this machine with the port for game.
        /// This method can not be called when hosting room because game port may be used.
        /// </summary>
        /// <param name="protocol">The protocol which is used for accept TCP connection of game</param>
        /// <param name="portNumber">The port number which is used for accept TCP connection of game</param>
        /// <exception cref="ClientErrorException"></exception>
        /// <exception cref="ClientInternalErrorException"></exception>
        /// <exception cref="InvalidOperationException">The port is already used by other connection which is not TCP server</exception>
        /// <returns></returns>
        public async Task<bool> ConnectionTestAsync(TransportProtocol protocol, ushort portNumber)
        {
            await semaphore.WaitAsync().ConfigureAwait(false);
            try
            {
                if (IsHostingRoom)
                {
                    throw new ClientErrorException(ClientErrorCode.AlreadyHostingRoom,
                        "The client can host only one room.");
                }

                return await ConnectionTestCoreAsync(protocol, portNumber).ConfigureAwait(false);
            }
            finally
            {
                semaphore.Release();
            }
        }

        public void Dispose()
        {
            tcpClient?.Dispose();
            semaphore.Dispose();
        }

        private TcpClient tcpClient;
        private readonly int timeoutMilliSeconds;
        private uint sessionKey;
        private readonly SemaphoreSlim semaphore = new SemaphoreSlim(1, 1);

        /// <summary>
        /// Send a request or notice message to the server.
        /// </summary>
        /// <typeparam name="T">A type of the message</typeparam>
        /// <param name="messageBody"></param>
        /// <exception cref="ClientErrorException"></exception>
        /// <exception cref="ClientInternalErrorException"></exception>
        /// <returns></returns>
        private async Task SendRequestAsync<T>(T messageBody)
        {
            try
            {
                await tcpClient.SendRequestMessage(messageBody, sessionKey).ConfigureAwait(false);
            }
            catch (MessageInternalErrorException e)
            {
                throw new ClientInternalErrorException(e.Message);
            }
            catch (ObjectDisposedException e)
            {
                OnConnectionClosed();
                throw new ClientErrorException(ClientErrorCode.ConnectionClosed, e.Message);
            }
            catch (SocketException e)
            {
                throw new ClientInternalErrorException(e.Message);
            }
        }

        /// <summary>
        /// Receive a reply message from the server.
        /// </summary>
        /// <typeparam name="T">A type of the message</typeparam>
        /// <exception cref="ClientErrorException"></exception>
        /// <exception cref="ClientInternalErrorException"></exception>
        /// <returns></returns>
        private async Task<T> ReceiveReplyAsync<T>()
        {
            try
            {
                var (errorCode, replyBody) = await tcpClient.ReceiveReplyMessage<T>().ConfigureAwait(false);
                if (errorCode != MessageErrorCode.Ok)
                {
                    throw new ClientErrorException(ClientErrorCode.RequestError, errorCode.ToString());
                }

                return replyBody;
            }
            catch (MessageInternalErrorException e)
            {
                throw new ClientInternalErrorException(e.Message);
            }
            catch (ObjectDisposedException e)
            {
                OnConnectionClosed();
                throw new ClientErrorException(ClientErrorCode.ConnectionClosed, e.Message);
            }
            catch (SocketException e)
            {
                throw new ClientInternalErrorException(e.Message);
            }
        }

        /// <summary>
        /// Create and host new room to the server without try block and parameter validations.
        /// </summary>
        /// <param name="portNumber">The port number which is used for accept TCP connection of game</param>
        /// <param name="roomGroupIndex"></param>
        /// <param name="roomName"></param>
        /// <param name="maxPlayerCount"></param>
        /// <param name="isPublic"></param>
        /// <param name="password"></param>
        /// <exception cref="ClientErrorException"></exception>
        /// <exception cref="ClientInternalErrorException"></exception>
        /// <exception cref="ArgumentException"></exception>
        /// <returns></returns>
        private async Task CreateRoomCoreAsync(ushort portNumber, byte roomGroupIndex, string roomName,
            byte maxPlayerCount, bool isPublic = true, string password = "")
        {
            var requestBody = new CreateRoomRequestMessage
            {
                GroupIndex = roomGroupIndex,
                Name = roomName,
                Password = password,
                MaxPlayerCount = maxPlayerCount,
                IsPublic = isPublic,
                portNumber = portNumber
            };
            await SendRequestAsync(requestBody).ConfigureAwait(false);
            Logger.Log(LogLevel.Info,
                $"Send CreateRoomRequest. ({nameof(requestBody.GroupIndex)}: {requestBody.GroupIndex}, {nameof(requestBody.Name)}: {requestBody.Name}, {nameof(requestBody.Password)}: {requestBody.Password}, {nameof(requestBody.MaxPlayerCount)}: {requestBody.MaxPlayerCount}, {nameof(requestBody.IsPublic)}: {requestBody.IsPublic}, {nameof(requestBody.portNumber)}: {requestBody.portNumber})");

            var replyBody = await ReceiveReplyAsync<CreateRoomReplyMessage>().ConfigureAwait(false);
            Logger.Log(LogLevel.Info, $"Receive CreateRoomReply. ({nameof(replyBody.RoomId)}: {replyBody.RoomId})");
            IsHostingRoom = true;
            HostingRoomGroupIndex = roomGroupIndex;
            HostingRoomId = replyBody.RoomId;
        }

        /// <summary>
        /// Check if other machine can connect to this machine with the port for game without semaphore.
        /// </summary>
        /// <param name="protocol">The protocol which is used for accept TCP connection of game</param>
        /// <param name="portNumber">The port number which is used for accept TCP connection of game</param>
        /// <exception cref="ClientErrorException"></exception>
        /// <exception cref="ClientInternalErrorException"></exception>
        /// <exception cref="InvalidOperationException">The port is already used by other connection or listener</exception>
        /// <returns></returns>
        private async Task<bool> ConnectionTestCoreAsync(TransportProtocol protocol, ushort portNumber)
        {
            TcpListener tcpListener = null;
            UdpClient udpClient = null;
            Task task = null;
            var cancelTokenSource = new CancellationTokenSource();
            try
            {
                if (!Connected)
                {
                    throw new ClientErrorException(ClientErrorCode.NotConnected);
                }

                if (!NetworkHelper.CheckPortAvailability(protocol, portNumber))
                {
                    throw new InvalidOperationException(
                        $"The port \"{portNumber}\" is already used by other {portNumber} connection or listener.");
                }

                // Accept both IPv4 and IPv6
                if (protocol == TransportProtocol.Tcp)
                {
                    tcpListener = new TcpListener(IPAddress.IPv6Any, portNumber);
                    tcpListener.Server.SetSocketOption(
                        SocketOptionLevel.IPv6,
                        SocketOptionName.IPv6Only,
                        0);
                    tcpListener.Start();

                    async Task Func(CancellationToken cancellationToken)
                    {
                        try
                        {
                            while (!cancellationToken.IsCancellationRequested)
                            {
                                using (await Task.Run(tcpListener.AcceptSocketAsync, cancellationToken)
                                    .ConfigureAwait(false))
                                {
                                }
                            }
                        }
                        // ObjectDisposedException is thrown when we cancel accepting after we started accepting.
                        catch (ObjectDisposedException)
                        {
                        }
                    }

                    task = Func(cancelTokenSource.Token);
                }
                else
                {
                    udpClient = new UdpClient(portNumber);

                    async Task Func(UdpClient pUdpClient, CancellationToken cancellationToken)
                    {
                        try
                        {
                            var serverAddress = ((IPEndPoint)tcpClient.Client.RemoteEndPoint).Address;
                            while (true)
                            {
                                var receiveResult = await Task.Run(pUdpClient.ReceiveAsync, cancellationToken)
                                    .ConfigureAwait(false);
                                if (cancellationToken.IsCancellationRequested)
                                {
                                    return;
                                }

                                // reply only if the message is from the server
                                if (Equals(receiveResult.RemoteEndPoint.Address, serverAddress))
                                {
                                    await Task.Run(async () =>
                                    {
                                        await pUdpClient.SendAsync(receiveResult.Buffer, receiveResult.Buffer.Length,
                                            receiveResult.RemoteEndPoint).ConfigureAwait(false);
                                    }, cancellationToken).ConfigureAwait(false);
                                }
                            }
                        }
                        // ObjectDisposedException is thrown when we cancel accepting after we started accepting.
                        catch (ObjectDisposedException)
                        {
                        }
                    }

                    task = Func(udpClient, cancelTokenSource.Token);
                }

                var requestBody = new ConnectionTestRequestMessage {Protocol = protocol, PortNumber = portNumber};
                Logger.Log(LogLevel.Info,
                    $"Send ConnectionTestRequest. ({nameof(requestBody.Protocol)}: {requestBody.Protocol}, {nameof(requestBody.PortNumber)}: {requestBody.PortNumber})");
                await SendRequestAsync(requestBody).ConfigureAwait(false);
                var reply = await ReceiveReplyAsync<ConnectionTestReplyMessage>().ConfigureAwait(false);
                Logger.Log(LogLevel.Info, $"Receive ConnectionTestReply. ({nameof(reply.Succeed)}: {reply.Succeed})");
                return reply.Succeed;
            }
            catch (ClientInternalErrorException)
            {
                if (tcpClient.Connected)
                {
                    Close();
                }

                throw;
            }
            finally
            {
                cancelTokenSource.Cancel();
                tcpListener?.Stop();
                udpClient?.Close();
                if (task != null)
                {
                    await Task.WhenAll(task).ConfigureAwait(false);
                }

                task?.Dispose();
                udpClient?.Dispose();
                cancelTokenSource.Dispose();
            }
        }

        private TcpClient CreateTcpClient()
        {
            var newTcpClient =
                new TcpClient {ReceiveTimeout = timeoutMilliSeconds, SendBufferSize = timeoutMilliSeconds};
            newTcpClient.Client.SetSocketOption(SocketOptionLevel.Socket, SocketOptionName.ReuseAddress, true);
            return newTcpClient;
        }

        private void OnConnectionClosed()
        {
            IsHostingRoom = false;
        }
    }
}
#pragma warning restore CA1303