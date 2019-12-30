using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Net.NetworkInformation;
using System.Net.Sockets;
using System.Reflection;
using System.Threading;
using System.Threading.Tasks;
using CdecPGL.MinimalSerializer;

namespace PlanetaGameLabo.MatchMaker
{
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
        public NatPortMappingCreator PortMappingCreator { get; } = new NatPortMappingCreator();

        public MatchMakerClient(ILogger logger = null)
        {
            if (logger == null)
            {
                logger = StreamLogger.CreateStandardOutputLogger();
            }

            this.logger = logger;
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
            await semaphore.WaitAsync();
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
                    tcpClient = new TcpClient();
                    tcpClient.Client.SetSocketOption(SocketOptionLevel.Socket, SocketOptionName.ReuseAddress, true);
                    await tcpClient.ConnectAsync(serverAddress, serverPort);
                    logger.Log(LogLevel.Info, $"Connect to {serverAddress}:{serverPort} successfully.");
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
                await SendRequestAsync(requestBody);
                logger.Log(LogLevel.Info,
                    $"Send AuthenticationRequest. ({nameof(ClientConstants.ApiVersion)}: {ClientConstants.ApiVersion})");
                var replyBody = await ReceiveReplyAsync<AuthenticationReplyMessage>();
                logger.Log(LogLevel.Info,
                    $"Receive AuthenticationReply. ({nameof(replyBody.SessionKey)}: {replyBody.SessionKey}, {nameof(replyBody.Version)}: {replyBody.Version})...");
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
            logger.Log(LogLevel.Info, $"Close connection to the server.");
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
            await semaphore.WaitAsync();
            try
            {
                if (!Connected)
                {
                    throw new ClientErrorException(ClientErrorCode.NotConnected);
                }

                var requestBody = new ListRoomGroupRequestMessage();
                await SendRequestAsync(requestBody);
                logger.Log(LogLevel.Info, $"Send ListRoomGroupRequest.");

                var replyBody = await ReceiveReplyAsync<ListRoomGroupReplyMessage>();
                logger.Log(LogLevel.Info,
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
        public async Task CreateRoomAsync(ushort portNumber, byte roomGroupIndex, string roomName, byte maxPlayerCount,
            bool isPublic = true, string password = "")
        {
            await semaphore.WaitAsync();
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

                await CreateRoomCoreAsync(portNumber, roomGroupIndex, roomName, maxPlayerCount, isPublic, password);
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
        /// </summary>
        /// <param name="discoverNatTimeoutMilliSeconds"></param>
        /// <param name="protocol">The protocol which is used for accept TCP connection of game</param>
        /// <param name="portNumberCandidates">The candidates of port number which is used for accept TCP connection of game</param>
        /// <param name="defaultPortNumber">The port number which is tried to use for accept TCP connection of game first</param>
        /// <param name="roomGroupIndex"></param>
        /// <param name="roomName"></param>
        /// <param name="maxPlayerCount"></param>
        /// <param name="isPublic"></param>
        /// <param name="password"></param>
        /// <exception cref="ClientErrorException"></exception>
        /// <exception cref="ClientInternalErrorException"></exception>
        /// <exception cref="ArgumentException"></exception>
        /// <returns></returns>
        public async Task CreateRoomAsyncWithCreatingPortMapping(int discoverNatTimeoutMilliSeconds,
            TransportProtocol protocol, IEnumerable<ushort> portNumberCandidates, ushort defaultPortNumber,
            byte roomGroupIndex, string roomName,
            byte maxPlayerCount, bool isPublic = true, string password = "")
        {
            await semaphore.WaitAsync();
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

                if (!PortMappingCreator.IsDiscoverNatDone)
                {
                    await PortMappingCreator.DiscoverNat(discoverNatTimeoutMilliSeconds);
                }

                if (!PortMappingCreator.IsNatDeviceAvailable)
                {
                    throw new ClientErrorException(ClientErrorCode.CreatingPortMappingFailed,
                        "Failed to discover NAT device.");
                }

                var connectionTestSucceed = await ConnectionTestCoreAsync(protocol, defaultPortNumber);
                var portNumber = defaultPortNumber;
                if (!connectionTestSucceed)
                {
                    var portNumberCandidateArray = portNumberCandidates.ToArray();
                    var ret = (await PortMappingCreator.CreatePortMappingFromCandidate(TransportProtocol.Tcp,
                        portNumberCandidateArray, portNumberCandidateArray, ""));
                    portNumber = ret.publicPort;
                    logger.Log(LogLevel.Info,
                        $"Port mapping is created in NAT. (privatePortNumber: {ret.privatePort}, publicPortNumber: {ret.publicPort})");
                }

                connectionTestSucceed = await ConnectionTestCoreAsync(protocol, portNumber);
                if (!connectionTestSucceed)
                {
                    throw new ClientErrorException(ClientErrorCode.NotReachable);
                }

                await CreateRoomCoreAsync(portNumber, roomGroupIndex, roomName, maxPlayerCount, isPublic, password);
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
        public async Task<(byte totalRoomCount, RoomResult[] roomInfoList)> GetRoomListAsync(
            byte roomGroupIndex, byte startIndex,
            byte count, RoomDataSortKind sortKind, RoomSearchTargetFlag searchTargetFlags = RoomSearchTargetFlag.All,
            string searchName = "")
        {
            await semaphore.WaitAsync();
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
                    EndIndex = (byte)(startIndex + count - 1),
                    SortKind = sortKind,
                    SearchTargetFlags = searchTargetFlags,
                    SearchName = searchName
                };
                await SendRequestAsync(requestBody);
                logger.Log(LogLevel.Info,
                    $"Send ListRoomRequest. ({requestBody.GroupIndex}: {requestBody.GroupIndex}, {requestBody.StartIndex}: {requestBody.StartIndex}, {requestBody.EndIndex}: {requestBody.EndIndex}, {requestBody.SortKind}: {requestBody.SortKind}, {requestBody.SearchTargetFlags}: {requestBody.SearchTargetFlags}, {requestBody.SearchName}: {requestBody.SearchName}");

                var replyBody = await ReceiveReplyAsync<ListRoomReplyMessage>();
                logger.Log(LogLevel.Info,
                    $"Receive ListRoomReply. (RoomCount: {replyBody.ResultRoomCount}");
                var result = new RoomResult[replyBody.ResultRoomCount];

                // Set results of reply to result list
                void SetResult(in ListRoomReplyMessage reply)
                {
                    for (var i = 0; i < reply.ReplyRoomCount; ++i)
                    {
                        result[reply.ReplyRoomStartIndex + i] = new RoomResult(reply.RoomInfoList[i]);
                    }
                }

                SetResult(replyBody);

                var separateCount = (replyBody.ResultRoomCount - 1) / RoomConstants.ListRoomReplyRoomInfoCount + 1;

                for (var i = 1; i < separateCount; ++i)
                {
                    replyBody = await ReceiveReplyAsync<ListRoomReplyMessage>();
                    logger.Log(LogLevel.Info, $"Receive additional ListRoomReply ({i + 1}/{separateCount}).");
                    SetResult(replyBody);
                }

                return (replyBody.TotalRoomCount, result);
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
        /// <returns></returns>
        public async Task<IPEndPoint> JoinRoomAsync(byte roomGroupIndex, uint roomId, string password = "")
        {
            await semaphore.WaitAsync();
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
                await SendRequestAsync(requestBody);
                logger.Log(LogLevel.Info,
                    $"Send JoinRoomRequest. ({nameof(requestBody.GroupIndex)}: {requestBody.GroupIndex}, {nameof(requestBody.RoomId)}: {requestBody.RoomId}, {nameof(requestBody.Password)}: {requestBody.Password})");

                var replyBody = await ReceiveReplyAsync<JoinRoomReplyMessage>();
                logger.Log(LogLevel.Info,
                    $"Receive JoinRoomReply. ({nameof(replyBody.HostAddress)}: {replyBody.HostAddress})");

                return (IPEndPoint)replyBody.HostAddress;
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
            await semaphore.WaitAsync();
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
                await SendRequestAsync(requestBody);
                logger.Log(LogLevel.Info,
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
            await UpdateHostingRoomStatusAsync(RoomStatus.Remove);
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
            await semaphore.WaitAsync();
            try
            {
                if (IsHostingRoom)
                {
                    throw new ClientErrorException(ClientErrorCode.AlreadyHostingRoom,
                        "The client can host only one room.");
                }

                return await ConnectionTestCoreAsync(protocol, portNumber);
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
        private readonly ILogger logger;

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
                await tcpClient.SendRequestMessage(messageBody, sessionKey);
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
                var (errorCode, replyBody) = await tcpClient.ReceiveReplyMessage<T>();
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
            await SendRequestAsync(requestBody);
            logger.Log(LogLevel.Info,
                $"Send CreateRoomRequest. ({nameof(requestBody.GroupIndex)}: {requestBody.GroupIndex}, {nameof(requestBody.Name)}: {requestBody.Name}, {nameof(requestBody.Password)}: {requestBody.Password}, {nameof(requestBody.MaxPlayerCount)}: {requestBody.MaxPlayerCount}, {nameof(requestBody.IsPublic)}: {requestBody.IsPublic}, {nameof(requestBody.portNumber)}: {requestBody.portNumber})");

            var replyBody = await ReceiveReplyAsync<CreateRoomReplyMessage>();
            logger.Log(LogLevel.Info, $"Receive CreateRoomReply. ({nameof(replyBody.RoomId)}: {replyBody.RoomId})");
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
        /// <exception cref="InvalidOperationException">The port is already used by other connection which is not TCP server</exception>
        /// <returns></returns>
        private async Task<bool> ConnectionTestCoreAsync(TransportProtocol protocol, ushort portNumber)
        {
            TcpListener tcpListener = null;
            UdpClient udpClient = null;
            Socket socket = null;
            Task task = null;
            var cancelTokenSource = new CancellationTokenSource();
            try
            {
                if (!Connected)
                {
                    throw new ClientErrorException(ClientErrorCode.NotConnected);
                }

                var ipGlobalProperties = IPGlobalProperties.GetIPGlobalProperties();

                // error if the port is already used by other connection
                if (ipGlobalProperties.GetActiveTcpConnections().Any(c => c.LocalEndPoint.Port == portNumber))
                {
                    throw new InvalidOperationException(
                        $"The port \"{portNumber}\" is already used by other connection which is not TCP server.");
                }

                // Accept both IPv4 and IPv6
                if (protocol == TransportProtocol.Tcp)
                {
                    tcpListener = new TcpListener(System.Net.IPAddress.IPv6Any, portNumber);
                    tcpListener.Server.SetSocketOption(
                        System.Net.Sockets.SocketOptionLevel.IPv6,
                        System.Net.Sockets.SocketOptionName.IPv6Only,
                        0);
                    tcpListener.Start();
                    Func<CancellationToken, Task> func = async cancellationToken =>
                    {
                        while (true)
                        {
                            socket = await Task.Run(tcpListener.AcceptSocketAsync, cancellationToken);
                            if (socket.Connected)
                            {
                                socket.Dispose();
                                socket = null;
                            }

                            if (cancellationToken.IsCancellationRequested)
                            {
                                return;
                            }
                        }
                    };
                    task = func.Invoke(cancelTokenSource.Token);
                }
                else
                {
                    udpClient = new UdpClient(portNumber);
                    Func<UdpClient, CancellationToken, Task> func = async (pUdpClient, cancellationToken) =>
                    {
                        var serverAddress = ((IPEndPoint)tcpClient.Client.RemoteEndPoint).Address;
                        while (true)
                        {
                            var receiveResult = await Task.Run(pUdpClient.ReceiveAsync, cancellationToken);
                            if (cancellationToken.IsCancellationRequested)
                            {
                                return;
                            }

                            // reply only if the message is from the server
                            if (Equals(receiveResult.RemoteEndPoint.Address, serverAddress))
                            {
                                await pUdpClient.SendAsync(receiveResult.Buffer, receiveResult.Buffer.Length,
                                    receiveResult.RemoteEndPoint);
                            }
                        }
                    };
                    task = func.Invoke(udpClient, cancelTokenSource.Token);
                }

                var requestBody = new ConnectionTestRequestMessage {Protocol = protocol, PortNumber = portNumber};
                logger.Log(LogLevel.Info,
                    $"Send ConnectionTestRequest. ({nameof(requestBody.Protocol)}: {requestBody.Protocol}, {nameof(requestBody.PortNumber)}: {requestBody.PortNumber})");
                await SendRequestAsync(requestBody);
                var reply = await ReceiveReplyAsync<ConnectionTestReplyMessage>();
                logger.Log(LogLevel.Info, $"Receive ConnectionTestReply. ({nameof(reply.Succeed)}: {reply.Succeed})");
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
                socket?.Dispose();
                task?.Dispose();
                tcpListener?.Stop();
                udpClient?.Dispose();
            }
        }

        private void OnConnectionClosed()
        {
            IsHostingRoom = false;
        }
    }
}