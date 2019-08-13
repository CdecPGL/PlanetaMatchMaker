using System;
using System.Linq;
using System.Net.Sockets;
using System.Threading.Tasks;

namespace PlanetaGameLabo.MatchMaker {
    public sealed class MatchMakerClient : IDisposable {
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
        /// <returns></returns>
        public async Task ConnectAsync(string serverAddress, ushort serverPort) {
            if (Connected) {
                throw new ClientErrorException(ClientErrorCode.AlreadyConnected);
            }

            try {
                tcpClient = new TcpClient();
                tcpClient.Client.SetSocketOption(SocketOptionLevel.Socket, SocketOptionName.ReuseAddress, true);
                await tcpClient.ConnectAsync(serverAddress, serverPort);
            }
            catch (SocketException e) {
                throw new ClientErrorException(ClientErrorCode.FailedToConnect, e.Message);
            }
            catch (ObjectDisposedException e) {
                throw new ClientErrorException(ClientErrorCode.FailedToConnect, e.Message);
            }

            var requestBody = new AuthenticationRequestMessage {Version = ClientConstants.ApiVersion};
            await SendRequestAsync(requestBody);

            var replyBody = await ReceiveReplyAsync<AuthenticationReplyMessage>();
            sessionKey = replyBody.SessionKey;
        }

        /// <summary>
        /// Close the connection.
        /// </summary>
        public void Close() {
            if (!Connected) {
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
        public async Task<ListRoomGroupReplyMessage.RoomGroupInfo[]> GetRoomGroupListAsync() {
            if (!Connected) {
                throw new ClientErrorException(ClientErrorCode.NotConnected);
            }

            var requestBody = new ListRoomGroupRequestMessage();
            await SendRequestAsync(requestBody);

            var replyBody = await ReceiveReplyAsync<ListRoomGroupReplyMessage>();
            return replyBody.RoomGroupInfoList.Take(replyBody.RoomGroupCount).ToArray();
        }

        /// <summary>
        /// Create and host new room to the server.
        /// </summary>
        /// <param name="roomGroupIndex"></param>
        /// <param name="roomName"></param>
        /// <exception cref="ClientErrorException"></exception>
        /// <returns></returns>
        public async Task CreateRoomAsync(byte roomGroupIndex, string roomName) {
            if (!Connected) {
                throw new ClientErrorException(ClientErrorCode.NotConnected);
            }

            if (IsHostingRoom) {
                throw new ClientErrorException(ClientErrorCode.AlreadyHostingRoom,
                    "The client can host only one room.");
            }

            var requestBody = new CreateRoomRequestMessage {
                GroupIndex = roomGroupIndex,
                Name = roomName
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
        /// <param name="flags"></param>
        /// <exception cref="ClientErrorException"></exception>
        /// <returns></returns>
        public async Task<(int totalRoomCount, ListRoomReplyMessage.RoomInfo[] roomInfoList)> GetRoomList(
            byte roomGroupIndex, byte startIndex,
            byte count, RoomDataSortKind sortKind, byte flags) {
            if (!Connected) {
                throw new ClientErrorException(ClientErrorCode.NotConnected);
            }

            var requestBody = new ListRoomRequestMessage {
                GroupIndex = roomGroupIndex,
                StartIndex = startIndex,
                EndIndex = (byte) (startIndex + count - 1),
                SortKind = sortKind,
                Flags = flags
            };
            await SendRequestAsync(requestBody);

            var replyBody = await ReceiveReplyAsync<ListRoomReplyMessage>();
            var result = new ListRoomReplyMessage.RoomInfo[replyBody.ResultRoomCount];

            // Set results of reply to result list
            void SetResult(in ListRoomReplyMessage reply) {
                for (var i = 0; i < reply.ReplyRoomEndIndex - reply.ReplyRoomStartIndex + 1; ++i) {
                    result[reply.ReplyRoomStartIndex + i] = reply.RoomInfoList[i];
                }
            }

            SetResult(replyBody);

            var separateCount = (replyBody.ResultRoomCount - 1) / ClientConstants.ListRoomReplyRoomInfoCount + 1;

            for (var i = 1; i < separateCount; ++i) {
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
        /// <exception cref="ClientErrorException"></exception>
        /// <returns></returns>
        public async Task<ClientAddress> JoinRoom(byte roomGroupIndex, uint roomId) {
            if (!Connected) {
                throw new ClientErrorException(ClientErrorCode.NotConnected);
            }

            if (IsHostingRoom) {
                throw new ClientErrorException(ClientErrorCode.AlreadyHostingRoom,
                    "The client hosting a room can't join the room.");
            }

            var requestBody = new JoinRoomRequestMessage {
                GroupIndex = roomGroupIndex,
                RoomId = roomId
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
        public async Task UpdateHostingRoomStatus(UpdateRoomStatusNoticeMessage.RoomStatus roomStatus) {
            if (!Connected) {
                throw new ClientErrorException(ClientErrorCode.NotConnected);
            }

            if (!IsHostingRoom) {
                throw new ClientErrorException(ClientErrorCode.NotHostingRoom);
            }

            var requestBody = new UpdateRoomStatusNoticeMessage {
                GroupIndex = hostingRoomGroupIndex,
                RoomId = hostingRoomId,
                Status = roomStatus
            };
            await SendRequestAsync(requestBody);

            if (roomStatus == UpdateRoomStatusNoticeMessage.RoomStatus.Remove) {
                IsHostingRoom = false;
            }
        }

        /// <summary>
        /// Remove hosting room from the server.
        /// </summary>
        /// <exception cref="ClientErrorException"></exception>
        /// <returns></returns>
        public async Task RemoveHostingRoom() {
            await UpdateHostingRoomStatus(UpdateRoomStatusNoticeMessage.RoomStatus.Remove);
        }

        public void Dispose() {
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
        private async Task SendRequestAsync<T>(T messageBody) {
            try {
                await tcpClient.SendRequestMessage(messageBody, sessionKey);
            }
            catch (MessageErrorException e) {
                throw new ClientErrorException(ClientErrorCode.MessageSendError, e.Message);
            }
            catch (ObjectDisposedException e) {
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
        private async Task<T> ReceiveReplyAsync<T>() {
            try {
                var (errorCode, replyBody) = await tcpClient.ReceiveReplyMessage<T>();
                if (errorCode != MessageErrorCode.Ok) {
                    throw new ClientErrorException(ClientErrorCode.RequestError, errorCode.ToString());
                }

                return replyBody;
            }
            catch (MessageErrorException e) {
                throw new ClientErrorException(ClientErrorCode.MessageReceptionError, e.Message);
            }
            catch (ObjectDisposedException e) {
                OnConnectionClosed();
                throw new ClientErrorException(ClientErrorCode.ConnectionClosed, e.Message);
            }
        }

        private void OnConnectionClosed() {
            IsHostingRoom = false;
        }
    }
}