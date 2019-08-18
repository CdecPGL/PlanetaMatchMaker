using System;
using System.Linq;
using System.Net.Sockets;
using System.Reflection;
using System.Threading.Tasks;
using PlanetaGameLabo.Serializer;

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
        /// Connect to matching server.
        /// </summary>
        /// <param name="serverAddress"></param>
        /// <param name="serverPort"></param>
        /// <exception cref="ClientErrorException"></exception>
        /// <exception cref="ArgumentException"></exception>
        /// <returns></returns>
        public async Task ConnectAsync(string serverAddress, ushort serverPort)
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

            var replyBody = await ReceiveReplyAsync<AuthenticationReplyMessage>();
            sessionKey = replyBody.SessionKey;
        }

        /// <summary>
        /// Close the connection.
        /// </summary>
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
            OnConnectionClosed();
        }

        /// <summary>
        /// Get a room group list from the server.
        /// </summary>
        /// <exception cref="ClientErrorException"></exception>
        /// <returns></returns>
        public async Task<RoomGroupResult[]> GetRoomGroupListAsync()
        {
            if (!Connected)
            {
                throw new ClientErrorException(ClientErrorCode.NotConnected);
            }

            var requestBody = new ListRoomGroupRequestMessage();
            await SendRequestAsync(requestBody);

            var replyBody = await ReceiveReplyAsync<ListRoomGroupReplyMessage>();
            return replyBody.RoomGroupInfoList.Take((int)replyBody.RoomGroupCount)
                .Select(info => new RoomGroupResult(info))
                .ToArray();
        }

        /// <summary>
        /// Create and host new room to the server.
        /// </summary>
        /// <param name="roomGroupIndex"></param>
        /// <param name="roomName"></param>
        /// <param name="password"></param>
        /// <exception cref="ClientErrorException"></exception>
        /// <exception cref="ArgumentException"></exception>
        /// <returns></returns>
        public async Task CreateRoomAsync(byte roomGroupIndex, string roomName, byte maxPlayerCount,
            string password = "")
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

            var requestBody = new CreateRoomRequestMessage
            {
                GroupIndex = roomGroupIndex, Name = roomName, Password = password, MaxPlayerCount = maxPlayerCount
            };
            await SendRequestAsync(requestBody);

            var replyBody = await ReceiveReplyAsync<CreateRoomReplyMessage>();
            IsHostingRoom = true;
            hostingRoomGroupIndex = roomGroupIndex;
            hostingRoomId = replyBody.RoomId;
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
        /// <returns></returns>
        public async Task<(int totalRoomCount, RoomResult[] roomInfoList)> GetRoomListAsync(
            byte roomGroupIndex, byte startIndex,
            byte count, RoomDataSortKind sortKind, RoomSearchTargetFlag searchTargetFlags = RoomSearchTargetFlag.All,
            string searchName = "")
        {
            if (searchName == null)
            {
                throw new ArgumentNullException(nameof(searchName));
            }

            var searchNameMaxLength = typeof(ListRoomRequestMessage).GetField(nameof(ListRoomRequestMessage.SearchName))
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

            var replyBody = await ReceiveReplyAsync<ListRoomReplyMessage>();
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
                SetResult(replyBody);
            }

            return (replyBody.TotalRoomCount, result);
        }

        /// <summary>
        /// Join to a room on the server.
        /// </summary>
        /// <param name="roomGroupIndex"></param>
        /// <param name="roomId"></param>
        /// <param name="password"></param>
        /// <exception cref="ClientErrorException"></exception>
        /// <returns></returns>
        public async Task<ClientAddress> JoinRoomAsync(byte roomGroupIndex, uint roomId, string password = "")
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

            var replyBody = await ReceiveReplyAsync<JoinRoomReplyMessage>();
            return replyBody.HostAddress;
        }

        /// <summary>
        /// Update hosting room status on the server.
        /// </summary>
        /// <param name="roomStatus"></param>
        /// <exception cref="ClientErrorException"></exception>
        /// <returns></returns>
        public async Task UpdateHostingRoomStatusAsync(RoomStatus roomStatus)
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
                GroupIndex = hostingRoomGroupIndex, RoomId = hostingRoomId, Status = roomStatus
            };
            await SendRequestAsync(requestBody);

            if (roomStatus == RoomStatus.Remove)
            {
                IsHostingRoom = false;
            }
        }

        /// <summary>
        /// Remove hosting room from the server.
        /// </summary>
        /// <exception cref="ClientErrorException"></exception>
        /// <returns></returns>
        public async Task RemoveHostingRoomAsync()
        {
            await UpdateHostingRoomStatusAsync(RoomStatus.Remove);
        }

        public void Dispose()
        {
            tcpClient?.Dispose();
        }

        private TcpClient tcpClient;
        private uint sessionKey;
        private byte hostingRoomGroupIndex;
        private uint hostingRoomId;

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
                await tcpClient.SendRequestMessage(messageBody, sessionKey);
            }
            catch (MessageErrorException e)
            {
                throw new ClientErrorException(ClientErrorCode.MessageSendError, e.Message);
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
            catch (MessageErrorException e)
            {
                throw new ClientErrorException(ClientErrorCode.MessageReceptionError, e.Message);
            }
            catch (ObjectDisposedException e)
            {
                OnConnectionClosed();
                throw new ClientErrorException(ClientErrorCode.ConnectionClosed, e.Message);
            }
        }

        private void OnConnectionClosed()
        {
            IsHostingRoom = false;
        }
    }
}