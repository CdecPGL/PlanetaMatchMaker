using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Net.Sockets;
using System.Threading;
using System.Threading.Tasks;

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
        /// Hosting room status. This is valid when hosting room.
        /// </summary>

        public RoomSettingFlag HostingRoomSettingFlags { get; private set; } = RoomSettingFlag.None;

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
        /// Timeout milli seconds to send and receive server.
        /// </summary>
        public int TimeoutMilliSeconds { get; }

        /// <summary>
        /// A full name of player. This is valid when connecting to the server.
        /// </summary>
        public PlayerFullName PlayerFullName { get; private set; }

        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="timeoutMilliSeconds">Timeout milli seconds for send and receive. Timeout of connect is not effected.</param>
        /// <param name="keepAliveNoticeIntervalSeconds"></param>
        /// <param name="logger"></param>
        public MatchMakerClient(int timeoutMilliSeconds = 10000, int keepAliveNoticeIntervalSeconds = 30,
            ILogger logger = null)
        {
            if (logger == null)
            {
                logger = StreamLogger.CreateStandardOutputLogger();
            }

            TimeoutMilliSeconds = timeoutMilliSeconds;
            Logger = logger;
            PortMappingCreator = new NatPortMappingCreator(logger);
            keepAliveSenderNotificator = new KeepAliveSenderNotificator(this, keepAliveNoticeIntervalSeconds);
        }

        /// <summary>
        /// Connect to matching server.
        /// </summary>
        /// <param name="serverAddress"></param>
        /// <param name="serverPort"></param>
        /// <param name="playerName"></param>
        /// <exception cref="ClientErrorException"></exception>
        /// <exception cref="ArgumentException"></exception>
        /// <returns></returns>
        public async Task<PlayerFullName> ConnectAsync(string serverAddress, ushort serverPort, string playerName)
        {
            if (!Validator.ValidateServerAddress(serverAddress))
            {
                throw new ArgumentException("IPv4, IPv6 or URL is available.", nameof(serverAddress));
            }

            if (!Validator.ValidateServerPort(serverPort))
            {
                throw new ArgumentException("0 is not available.", nameof(serverAddress));
            }

            if (!Validator.ValidatePlayerName(playerName))
            {
                throw new ArgumentException(
                    $"null string or string whose length is more than {ClientConstants.PlayerNameLength} is not available.",
                    nameof(serverAddress));
            }

            await semaphore.WaitAsync().ConfigureAwait(false);
            try
            {
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

                keepAliveSenderNotificator.UpdateLastRequestTime();

                // Authentication Request
                var requestBody =
                    new AuthenticationRequestMessage {Version = ClientConstants.ApiVersion, PlayerName = playerName};
                await SendRequestAsync(requestBody).ConfigureAwait(false);
                Logger.Log(LogLevel.Info,
                    $"Send AuthenticationRequest. ({nameof(ClientConstants.ApiVersion)}: {ClientConstants.ApiVersion}, {nameof(playerName)}: {playerName})");
                var replyBody = await ReceiveReplyAsync<AuthenticationReplyMessage>().ConfigureAwait(false);
                Logger.Log(LogLevel.Info,
                    $"Receive AuthenticationReply. ({nameof(replyBody.SessionKey)}: {replyBody.SessionKey}, {nameof(replyBody.Version)}: {replyBody.Version}, {nameof(replyBody.PlayerTag)}: {replyBody.PlayerTag})");
                sessionKey = replyBody.SessionKey;
                PlayerFullName = new PlayerFullName {Name = playerName, Tag = replyBody.PlayerTag};

                keepAliveSenderNotificator.StartKeepAliveProc();
                return PlayerFullName;
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

            keepAliveSenderNotificator.StopKeepAliveProc();
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
        /// <returns></returns>
        public async Task<ListRoomGroupResultItem[]> GetRoomGroupListAsync()
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
                    .Select(info => new ListRoomGroupResultItem(info))
                    .ToArray();
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
        /// <param name="maxPlayerCount"></param>
        /// <param name="portNumber">The port number which is used for accept TCP connection of game</param>
        /// <param name="password"></param>
        /// <exception cref="ClientErrorException"></exception>
        /// <exception cref="ArgumentException"></exception>
        /// <returns></returns>
        public async Task<CreateRoomResult> CreateRoomAsync(byte roomGroupIndex, byte maxPlayerCount, ushort portNumber,
            string password = "")
        {
            if (!Validator.ValidateGameHostPort(portNumber))
            {
                throw new ArgumentException("Dynamic/private port is available.", nameof(portNumber));
            }

            if (!Validator.ValidateRoomPassword(password))
            {
                throw new ArgumentException(
                    $"A string whose length is more than {RoomConstants.RoomPasswordLength} is not available.",
                    nameof(password));
            }

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
                        "The client can host only one room.");
                }

                return await CreateRoomCoreAsync(portNumber, roomGroupIndex, maxPlayerCount, password)
                    .ConfigureAwait(false);
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
        /// Refer NatPortMappingCreator.CreatePortMappingFromCandidatesAsync method if you want to know selection rule of port from candidates.
        /// If second connection test after port mapping is created is failed, this method throws error.
        /// </summary>
        /// <param name="roomGroupIndex"></param>
        /// <param name="maxPlayerCount"></param>
        /// <param name="discoverNatTimeoutMilliSeconds"></param>
        /// <param name="protocol">The protocol which is used for accept TCP connection of game</param>
        /// <param name="portNumberCandidates">The candidates of port number which is used for accept TCP connection of game</param>
        /// <param name="defaultPortNumber">The port number which is tried to use for accept TCP connection of game first</param>
        /// <param name="password"></param>
        /// <param name="forceToDiscoverNatDevice">force to discover NAT device even if NAT is already discovered if true</param>
        /// <exception cref="ClientErrorException"></exception>
        /// <exception cref="ArgumentException"></exception>
        /// <returns></returns>
        public async Task<CreateRoomWithCreatingPortMappingResult> CreateRoomWithCreatingPortMappingAsync(
            byte roomGroupIndex, byte maxPlayerCount, TransportProtocol protocol,
            IEnumerable<ushort> portNumberCandidates, ushort defaultPortNumber,
            int discoverNatTimeoutMilliSeconds = 5000, string password = "", bool forceToDiscoverNatDevice = false)
        {
            var portNumberCandidateArray = portNumberCandidates.ToList();
            if (portNumberCandidateArray.Any(
                portNumberCandidate => !Validator.ValidateGameHostPort(portNumberCandidate)))
            {
                throw new ArgumentException("Dynamic/private port is available.", nameof(portNumberCandidates));
            }

            if (!Validator.ValidateGameHostPort(defaultPortNumber))
            {
                throw new ArgumentException("Dynamic/private port is available.", nameof(defaultPortNumber));
            }

            if (!Validator.ValidateRoomPassword(password))
            {
                throw new ArgumentException(
                    $"A string whose length is more than {RoomConstants.RoomPasswordLength} is not available.",
                    nameof(password));
            }

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
                        "The client can host only one room.");
                }

                Logger.Log(LogLevel.Info, $"Execute first connection test (Default port: {defaultPortNumber}).");
                var connectionTestSucceed = false;
                try
                {
                    connectionTestSucceed =
                        await ConnectionTestCoreAsync(protocol, defaultPortNumber).ConfigureAwait(false);
                }
                // Consider port already used error as connection test failure
                catch (InvalidOperationException e)
                {
                    Logger.Log(LogLevel.Info, $"Failed to listen the port {defaultPortNumber}. ({e.Message})");
                }

                var isDefaultPortUsed = true;
                ushort usedPrivatePortFromCandidates = 0;
                ushort usedPublicPortFromCandidates = 0;
                var portNumber = defaultPortNumber;

                if (!connectionTestSucceed)
                {
                    // Discover NAT device if need
                    if (forceToDiscoverNatDevice || !PortMappingCreator.IsNatDeviceAvailable ||
                        !PortMappingCreator.IsDiscoverNatDone)
                    {
                        Logger.Log(LogLevel.Info, "Execute discovering NAT device because it is not done.");
                        await PortMappingCreator.DiscoverNatAsync(discoverNatTimeoutMilliSeconds).ConfigureAwait(false);
                    }

                    if (!PortMappingCreator.IsNatDeviceAvailable)
                    {
                        throw new ClientErrorException(ClientErrorCode.CreatingPortMappingFailed,
                            "Failed to discover NAT device.");
                    }

                    Logger.Log(LogLevel.Info, "Try to create port mapping because connection test is failed.");
                    // Create port candidates
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
                        .CreatePortMappingFromCandidatesAsync(protocol, privatePortCandidates, publicPortCandidates)
                        .ConfigureAwait(false);
                    portNumber = usedPublicPortFromCandidates;
                    Logger.Log(LogLevel.Info,
                        $"Port mapping is created or reused in NAT. (privatePortNumber: {usedPrivatePortFromCandidates}, publicPortNumber: {usedPublicPortFromCandidates})");

                    Logger.Log(LogLevel.Info, "Execute second connection test.");

                    connectionTestSucceed = await ConnectionTestCoreAsync(protocol, portNumber).ConfigureAwait(false);
                    if (!connectionTestSucceed)
                    {
                        throw new ClientErrorException(ClientErrorCode.NotReachable);
                    }

                    Logger.Log(LogLevel.Info, "Second connection test is succeeded. Start to create room.");
                }

                var createRoomResult = await CreateRoomCoreAsync(portNumber, roomGroupIndex, maxPlayerCount, password)
                    .ConfigureAwait(false);

                return new CreateRoomWithCreatingPortMappingResult(isDefaultPortUsed, usedPrivatePortFromCandidates,
                    usedPublicPortFromCandidates, createRoomResult);
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
        /// <param name="searchTag"></param>
        /// <exception cref="ClientErrorException"></exception>
        /// <exception cref="ArgumentException"></exception>
        /// <returns></returns>
        public async Task<(byte totalRoomCount, byte matchedRoomCount, ListRoomResultItem[] roomInfoList)>
            GetRoomListAsync(
                byte roomGroupIndex, byte startIndex,
                byte count, RoomDataSortKind sortKind,
                RoomSearchTargetFlag searchTargetFlags = RoomSearchTargetFlag.All,
                string searchName = "", ushort searchTag = PlayerFullName.NotAssignedTag)
        {
            if (!Validator.ValidateSearchName(searchName))
            {
                throw new ArgumentException(
                    $"null string or string whose length is more than {ClientConstants.PlayerNameLength} is not available.",
                    nameof(searchName));
            }

            await semaphore.WaitAsync().ConfigureAwait(false);
            try
            {
                var searchFullName = new PlayerFullName() {Name = searchName, Tag = searchTag};

                if (searchName.Length > ClientConstants.PlayerNameLength)
                {
                    throw new ArgumentNullException(
                        $"The length of {nameof(searchName)} must be less than {ClientConstants.PlayerNameLength}.");
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
                    SearchFullName = searchFullName
                };
                await SendRequestAsync(requestBody).ConfigureAwait(false);
                Logger.Log(LogLevel.Info,
                    $"Send ListRoomRequest. ({nameof(requestBody.GroupIndex)}: {requestBody.GroupIndex}, {nameof(requestBody.StartIndex)}: {requestBody.StartIndex}, {nameof(requestBody.Count)}: {requestBody.Count}, {nameof(requestBody.SortKind)}: {requestBody.SortKind}, {nameof(requestBody.SearchTargetFlags)}: {requestBody.SearchTargetFlags}, {nameof(requestBody.SearchFullName)}: {requestBody.SearchFullName})");

                var replyBody = await ReceiveReplyAsync<ListRoomReplyMessage>().ConfigureAwait(false);
                Logger.Log(LogLevel.Info,
                    $"Receive ListRoomReply. ({nameof(replyBody.TotalRoomCount)}: {replyBody.TotalRoomCount}, {nameof(replyBody.MatchedRoomCount)}: {replyBody.MatchedRoomCount}, {nameof(replyBody.ReplyRoomCount)}: {replyBody.ReplyRoomCount})");
                var result = new ListRoomResultItem[replyBody.ReplyRoomCount];

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

                        result[currentIndex++] = new ListRoomResultItem(reply.RoomInfoList[i]);
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
        /// <exception cref="ArgumentException"></exception>
        /// <returns>Game host endpoint</returns>
        public async Task<IPEndPoint> JoinRoomAsync(byte roomGroupIndex, uint roomId, string password = "")
        {
            if (!Validator.ValidateRoomPassword(password))
            {
                throw new ArgumentException(
                    $"A string whose length is more than {RoomConstants.RoomPasswordLength} is not available.",
                    nameof(password));
            }

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

                switch (roomStatus)
                {
                    case RoomStatus.Remove:
                        IsHostingRoom = false;
                        HostingRoomId = 0;
                        HostingRoomSettingFlags = RoomSettingFlag.None;
                        break;
                    case RoomStatus.Open:
                        HostingRoomSettingFlags |= RoomSettingFlag.OpenRoom;
                        break;
                    case RoomStatus.Close:
                        HostingRoomSettingFlags &= ~RoomSettingFlag.OpenRoom;
                        break;
                    default:
                        throw new ArgumentOutOfRangeException(nameof(roomStatus), roomStatus, null);
                }
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
        /// <returns></returns>
        public async Task RemoveHostingRoomAsync()
        {
            await UpdateHostingRoomStatusAsync(RoomStatus.Remove).ConfigureAwait(false);
        }

        /// <summary>
        /// Update current player count of hosting room.
        /// </summary>
        /// <exception cref="ClientErrorException"></exception>
        /// <returns></returns>
        public async Task UpdateHostingRoomCurrentPlayerCountAsync(byte currentPlayerCount = 0)
        {
            var isOpen = (HostingRoomSettingFlags | RoomSettingFlag.OpenRoom) == RoomSettingFlag.OpenRoom;
            await UpdateHostingRoomStatusAsync(isOpen ? RoomStatus.Open : RoomStatus.Close, true, currentPlayerCount)
                .ConfigureAwait(false);
        }

        /// <summary>
        /// Check if other machine can connect to this machine with the port for game.
        /// This method can not be called when hosting room because game port may be used.
        /// </summary>
        /// <param name="protocol">The protocol which is used for accept TCP connection of game</param>
        /// <param name="portNumber">The port number which is used for accept TCP connection of game</param>
        /// <exception cref="ClientErrorException"></exception>
        /// <exception cref="InvalidOperationException">The port is already used by other connection which is not TCP server</exception>
        /// <exception cref="ArgumentException"></exception>
        /// <returns></returns>
        public async Task<bool> ConnectionTestAsync(TransportProtocol protocol, ushort portNumber)
        {
            if (!Validator.ValidateGameHostPort(portNumber))
            {
                throw new ArgumentException("Dynamic/private port is available.", nameof(portNumber));
            }

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

        /// <summary>
        /// Notify alive of this client to the server.
        /// </summary>
        /// <exception cref="ClientErrorException"></exception>
        /// <returns></returns>
        public async Task NoticeAliveToTheServerAsync()
        {
            await semaphore.WaitAsync().ConfigureAwait(false);
            try
            {
                if (!Connected)
                {
                    throw new ClientErrorException(ClientErrorCode.NotConnected);
                }

                var requestBody = new KeepAliveNoticeMessage();
                await SendRequestAsync(requestBody).ConfigureAwait(false);
                Logger.Log(LogLevel.Info, "Send KeepAliveNotice.");
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
        private uint sessionKey;
        private readonly SemaphoreSlim semaphore = new SemaphoreSlim(1, 1);
        private readonly KeepAliveSenderNotificator keepAliveSenderNotificator;

        /// <summary>
        /// Send a request or notice message to the server.
        /// </summary>
        /// <typeparam name="T">A type of the message</typeparam>
        /// <param name="messageBody"></param>
        /// <exception cref="ClientErrorException"></exception>
        /// <returns></returns>
        private async Task SendRequestAsync<T>(T messageBody)
        {
            try
            {
                keepAliveSenderNotificator.UpdateLastRequestTime();
                await tcpClient.SendRequestMessage(messageBody, sessionKey).ConfigureAwait(false);
            }
            catch (MessageErrorException e)
            {
                if (Connected)
                {
                    Close();
                }

                throw new ClientErrorException(ClientErrorCode.SystemError, e.Message);
            }
            catch (ObjectDisposedException e)
            {
                OnConnectionClosed();
                throw new ClientErrorException(ClientErrorCode.ConnectionClosed, e.Message);
            }
            catch (SocketException e)
            {
                if (Connected)
                {
                    Close();
                }

                throw new ClientErrorException(ClientErrorCode.SystemError, e.Message);
            }
        }

        /// <summary>
        /// Receive a reply message from the server.
        /// </summary>
        /// <typeparam name="T">A type of the message</typeparam>
        /// <exception cref="ClientErrorException"></exception>
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
            catch (MessageErrorException e)
            {
                if (Connected)
                {
                    Close();
                }

                throw new ClientErrorException(ClientErrorCode.SystemError, e.Message);
            }
            catch (ObjectDisposedException e)
            {
                OnConnectionClosed();
                throw new ClientErrorException(ClientErrorCode.ConnectionClosed, e.Message);
            }
            catch (SocketException e)
            {
                if (Connected)
                {
                    Close();
                }

                throw new ClientErrorException(ClientErrorCode.SystemError, e.Message);
            }
        }

        /// <summary>
        /// Create and host new room to the server without try block and parameter validations.
        /// </summary>
        /// <param name="portNumber">The port number which is used for accept TCP connection of game</param>
        /// <param name="roomGroupIndex"></param>
        /// <param name="maxPlayerCount"></param>
        /// <param name="password"></param>
        /// <exception cref="ClientErrorException"></exception>
        /// <exception cref="ArgumentException"></exception>
        /// <returns></returns>
        private async Task<CreateRoomResult> CreateRoomCoreAsync(ushort portNumber, byte roomGroupIndex,
            byte maxPlayerCount,
            string password = "")
        {
            var requestBody = new CreateRoomRequestMessage
            {
                GroupIndex = roomGroupIndex,
                Password = password,
                MaxPlayerCount = maxPlayerCount,
                portNumber = portNumber
            };
            await SendRequestAsync(requestBody).ConfigureAwait(false);
            Logger.Log(LogLevel.Info,
                $"Send CreateRoomRequest. ({nameof(requestBody.GroupIndex)}: {requestBody.GroupIndex}, {nameof(requestBody.Password)}: {requestBody.Password}, {nameof(requestBody.MaxPlayerCount)}: {requestBody.MaxPlayerCount}, {nameof(requestBody.portNumber)}: {requestBody.portNumber})");

            var replyBody = await ReceiveReplyAsync<CreateRoomReplyMessage>().ConfigureAwait(false);
            Logger.Log(LogLevel.Info, $"Receive CreateRoomReply. ({nameof(replyBody.RoomId)}: {replyBody.RoomId})");
            // In server, the room  is created as open room and which has 1 player.
            IsHostingRoom = true;
            HostingRoomGroupIndex = roomGroupIndex;
            HostingRoomId = replyBody.RoomId;
            HostingRoomSettingFlags =
                (string.IsNullOrEmpty(password) ? RoomSettingFlag.PublicRoom : RoomSettingFlag.None) &
                RoomSettingFlag.OpenRoom;
            return new CreateRoomResult(HostingRoomId, HostingRoomSettingFlags, maxPlayerCount, 1);
        }

        /// <summary>
        /// Check if other machine can connect to this machine with the port for game without semaphore.
        /// </summary>
        /// <param name="protocol">The protocol which is used for accept TCP connection of game</param>
        /// <param name="portNumber">The port number which is used for accept TCP connection of game</param>
        /// <exception cref="ClientErrorException"></exception>
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

                Logger.Log(LogLevel.Info,
                    $"Start {protocol} connectable test to my port {portNumber} from the server.");

                // Accept both IPv4 and IPv6
                if (protocol == TransportProtocol.Tcp)
                {
                    tcpListener = new TcpListener(IPAddress.IPv6Any, portNumber);
                    tcpListener.Server.SetSocketOption(
                        SocketOptionLevel.IPv6,
                        SocketOptionName.IPv6Only,
                        0);
                    tcpListener.Start();
                    Logger.Log(LogLevel.Debug, "TCP listener for connectable test is started.");

                    async Task Func(CancellationToken cancellationToken)
                    {
                        try
                        {
                            using (var socket = await Task.Run(tcpListener.AcceptSocketAsync, cancellationToken)
                                .ConfigureAwait(false))
                            {
                                // Echo received message
                                var buffer = new ArraySegment<byte>(new byte[64]);
                                var size = await Task
                                    .Run(
                                        async () => await socket.ReceiveAsync(buffer, SocketFlags.None)
                                            .ConfigureAwait(false), cancellationToken).ConfigureAwait(false);
                                Logger.Log(LogLevel.Debug,
                                    $"A test message from the server is received: \"{System.Text.Encoding.ASCII.GetString(buffer.ToArray())}\"");

                                var replyData = new ArraySegment<byte>(buffer.ToArray(), 0, size);
                                await Task.Run(async () =>
                                {
                                    await socket.SendAsync(replyData, SocketFlags.None).ConfigureAwait(false);
                                }, cancellationToken).ConfigureAwait(false);
                                Logger.Log(LogLevel.Debug, "The received message is sent to the server.");

                                socket.Shutdown(SocketShutdown.Both);
                                socket.Disconnect(false);
                                Logger.Log(LogLevel.Debug, "TCP listener for connectable test is shut-downed.");
                            }
                        }
                        // ObjectDisposedException is thrown when we cancel accepting after we started accepting.
                        catch (ObjectDisposedException)
                        {
                            Logger.Log(LogLevel.Debug, "TCP listener for connectable test is disposed.");
                        }
                    }

                    task = Func(cancelTokenSource.Token);
                }
                else
                {
                    udpClient = new UdpClient(portNumber);
                    Logger.Log(LogLevel.Debug, "TCP client for connectable test is created.");

                    async Task Func(UdpClient pUdpClient, CancellationToken cancellationToken)
                    {
                        try
                        {
                            var serverAddress = ((IPEndPoint)tcpClient.Client.RemoteEndPoint).Address;
                            while (true)
                            {
                                var receiveResult = await Task.Run(pUdpClient.ReceiveAsync, cancellationToken)
                                    .ConfigureAwait(false);
                                Logger.Log(LogLevel.Debug,
                                    $"A message is received: \"{System.Text.Encoding.ASCII.GetString(receiveResult.Buffer)}\"");
                                if (cancellationToken.IsCancellationRequested)
                                {
                                    return;
                                }

                                // reply only if the message is from the server
                                if (receiveResult.RemoteEndPoint.Address.EqualsIpAddressSource(serverAddress))
                                {
                                    // Echo received message
                                    await Task.Run(async () =>
                                    {
                                        await pUdpClient.SendAsync(receiveResult.Buffer, receiveResult.Buffer.Length,
                                            receiveResult.RemoteEndPoint).ConfigureAwait(false);
                                    }, cancellationToken).ConfigureAwait(false);
                                    Logger.Log(LogLevel.Debug, "The received message is sent to the server.");
                                }
                                else
                                {
                                    Logger.Log(LogLevel.Debug,
                                        $"The sender ({receiveResult.RemoteEndPoint.Address}) of the message is not the server ({serverAddress}), so this message is ignored.");
                                }
                            }
                        }
                        // ObjectDisposedException is thrown when we cancel accepting after we started accepting.
                        catch (ObjectDisposedException)
                        {
                            Logger.Log(LogLevel.Debug, "UDP client for connectable test is disposed.");
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
                new TcpClient {ReceiveTimeout = TimeoutMilliSeconds, SendTimeout = TimeoutMilliSeconds};
            newTcpClient.Client.SetSocketOption(SocketOptionLevel.Socket, SocketOptionName.ReuseAddress, true);
            return newTcpClient;
        }

        private void OnConnectionClosed()
        {
            IsHostingRoom = false;
            HostingRoomId = 0;
            HostingRoomSettingFlags = RoomSettingFlag.None;
        }

        private sealed class KeepAliveSenderNotificator : IDisposable
        {
            public KeepAliveSenderNotificator(MatchMakerClient client, int keepAliveNoticeIntervalSeconds)
            {
                this.client = client;
                this.keepAliveNoticeIntervalSeconds = keepAliveNoticeIntervalSeconds;
            }

            public void StartKeepAliveProc()
            {
                cancellationTokenSource = new CancellationTokenSource();
                task = KeepAliveProcAsync(cancellationTokenSource.Token);
            }

            public void StopKeepAliveProc()
            {
                cancellationTokenSource.Cancel();
                cancellationTokenSource.Dispose();
                cancellationTokenSource = null;
            }

            public void UpdateLastRequestTime()
            {
                Interlocked.Exchange(ref lastRequestUnixTime, GetCurrentUnixTime());
            }

            private readonly MatchMakerClient client;
            private long lastRequestUnixTime;
            private CancellationTokenSource cancellationTokenSource;
            private Task task;
            private readonly int keepAliveNoticeIntervalSeconds;

            private static long GetCurrentUnixTime()
            {
                return new DateTimeOffset(DateTime.Now).ToUnixTimeSeconds();
            }

            private async Task KeepAliveProcAsync(CancellationToken cancellationToken)
            {
                while (!cancellationToken.IsCancellationRequested)
                {
                    var currentUnixTime = GetCurrentUnixTime();
                    var lastRequestUnixTimeRef = Interlocked.Read(ref lastRequestUnixTime);
                    try
                    {
                        await Task.Delay(1, cancellationToken).ConfigureAwait(false);
                    }
                    catch (OperationCanceledException)
                    {
                        break;
                    }

                    if (currentUnixTime - lastRequestUnixTimeRef < keepAliveNoticeIntervalSeconds)
                    {
                        continue;
                    }

                    UpdateLastRequestTime();
                    await client.NoticeAliveToTheServerAsync().ConfigureAwait(false);
                }
            }

            public void Dispose()
            {
                cancellationTokenSource?.Dispose();
                task?.Dispose();
            }
        }
    }
}
#pragma warning restore CA1303