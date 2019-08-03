using System;
using System.Linq;
using System.Net.Sockets;
using System.Threading.Tasks;

namespace PlanetaGameLabo.MatchMaker {
    public sealed class MatchMakerClient {
        /// <summary>
        /// true if connected to the server.
        /// </summary>
        public bool connected => _tcpClient.Connected;

        /// <summary>
        /// true if this client hosting a room.
        /// </summary>
        public bool isHostingRoom { get; private set; }

        /// <summary>
        /// Connect to matching server.
        /// </summary>
        /// <param name="server_address"></param>
        /// <param name="server_port"></param>
        /// <exception cref="ClientErrorException"></exception>
        /// <returns></returns>
        public async Task ConnectAsync(string server_address, ushort server_port) {
            if (_tcpClient.Connected) {
                throw new ClientErrorException(ClientErrorCode.AlreadyConnected);
            }

            try {
                await _tcpClient.ConnectAsync(server_address, server_port);
            }
            catch (SocketException e) {
                throw new ClientErrorException(ClientErrorCode.FailedToConnect, e.Message);
            }
            catch (ObjectDisposedException e) {
                throw new ClientErrorException(ClientErrorCode.FailedToConnect, e.Message);
            }

            var request_body = new AuthenticationRequestMessage {version = ClientConstants.clientVersion};
            await SendRequestAsync(request_body);

            var reply_body = await ReceiveReplyAsync<AuthenticationReplyMessage>();
            _sessionKey = reply_body.sessionKey;
        }

        /// <summary>
        /// Close the connection.
        /// </summary>
        public void Close() {
            if (!_tcpClient.Connected) {
                throw new ClientErrorException(ClientErrorCode.NotConnected);
            }

            _tcpClient.Close();
            OnConnectionClosed();
        }

        /// <summary>
        /// Get a room group list from the server.
        /// </summary>
        /// <exception cref="ClientErrorException"></exception>
        /// <returns></returns>
        public async Task<ListRoomGroupReplyMessage.RoomGroupInfo[]> GetRoomGroupListAsync() {
            if (!_tcpClient.Connected) {
                throw new ClientErrorException(ClientErrorCode.NotConnected);
            }

            var request_body = new ListRoomGroupRequestMessage();
            await SendRequestAsync(request_body);

            var reply_body = await ReceiveReplyAsync<ListRoomGroupReplyMessage>();
            return reply_body.roomGroupInfoList.Take(reply_body.roomGroupCount).ToArray();
        }

        /// <summary>
        /// Create and host new room to the server.
        /// </summary>
        /// <param name="room_group_index"></param>
        /// <param name="room_name"></param>
        /// <exception cref="ClientErrorException"></exception>
        /// <returns></returns>
        public async Task CreateRoomAsync(byte room_group_index, string room_name) {
            if (!_tcpClient.Connected) {
                throw new ClientErrorException(ClientErrorCode.NotConnected);
            }

            if (isHostingRoom) {
                throw new ClientErrorException(ClientErrorCode.AlreadyHostingRoom,
                    "The client can host only one room.");
            }

            var request_body = new CreateRoomRequestMessage {
                groupIndex = room_group_index,
                name = room_name
            };
            await SendRequestAsync(request_body);

            var reply_body = await ReceiveReplyAsync<CreateRoomReplyMessage>();
            isHostingRoom = true;
            _hostingRoomGroupIndex = room_group_index;
            _hostingRoomId = reply_body.roomId;
        }

        /// <summary>
        /// Get a room list from the server.
        /// </summary>
        /// <param name="room_group_index"></param>
        /// <param name="start_index"></param>
        /// <param name="count"></param>
        /// <param name="sort_kind"></param>
        /// <param name="flags"></param>
        /// <exception cref="ClientErrorException"></exception>
        /// <returns></returns>
        public async Task<(int totalRoomCount, ListRoomReplyMessage.RoomInfo[] roomInfoList)> GetRoomList(
            byte room_group_index, byte start_index,
            byte count, RoomDataSortKind sort_kind, byte flags) {
            if (!_tcpClient.Connected) {
                throw new ClientErrorException(ClientErrorCode.NotConnected);
            }

            var request_body = new ListRoomRequestMessage {
                groupIndex = room_group_index,
                startIndex = start_index,
                endIndex = (byte) (start_index + count - 1),
                sortKind = sort_kind,
                flags = flags
            };
            await SendRequestAsync(request_body);

            var reply_body = await ReceiveReplyAsync<ListRoomReplyMessage>();
            var result = new ListRoomReplyMessage.RoomInfo[reply_body.resultRoomCount];

            // Set results of reply to result list
            void SetResult(in ListRoomReplyMessage reply) {
                for (var i = 0; i < reply.replyRoomEndIndex - reply.replyRoomStartIndex + 1; ++i) {
                    result[reply.replyRoomStartIndex + i] = reply.roomInfoList[i];
                }
            }

            SetResult(reply_body);

            var separate_count = (reply_body.resultRoomCount - 1) / ClientConstants.listRoomReplyRoomInfoCount + 1;

            for (var i = 1; i < separate_count; ++i) {
                reply_body = await ReceiveReplyAsync<ListRoomReplyMessage>();
                SetResult(reply_body);
            }

            return (reply_body.totalRoomCount, result);
        }

        /// <summary>
        /// Join to a room on the server.
        /// </summary>
        /// <param name="room_group_index"></param>
        /// <param name="room_id"></param>
        /// <exception cref="ClientErrorException"></exception>
        /// <returns></returns>
        public async Task<ClientAddress> JoinRoom(byte room_group_index, uint room_id) {
            if (!_tcpClient.Connected) {
                throw new ClientErrorException(ClientErrorCode.NotConnected);
            }

            if (isHostingRoom) {
                throw new ClientErrorException(ClientErrorCode.AlreadyHostingRoom,
                    "The client hosting a room can't join the room.");
            }

            var request_body = new JoinRoomRequestMessage {
                groupIndex = room_group_index,
                roomId = room_id
            };
            await SendRequestAsync(request_body);

            var reply_body = await ReceiveReplyAsync<JoinRoomReplyMessage>();
            return reply_body.hostAddress;
        }

        /// <summary>
        /// Update hosting room status on the server.
        /// </summary>
        /// <param name="status"></param>
        /// <exception cref="ClientErrorException"></exception>
        /// <returns></returns>
        public async Task UpdateHostingRoomStatus(UpdateRoomStatusNoticeMessage.Status status) {
            if (!_tcpClient.Connected) {
                throw new ClientErrorException(ClientErrorCode.NotConnected);
            }

            if (!isHostingRoom) {
                throw new ClientErrorException(ClientErrorCode.NotHostingRoom);
            }

            var request_body = new UpdateRoomStatusNoticeMessage {
                groupIndex = _hostingRoomGroupIndex,
                roomId = _hostingRoomId,
                status = status
            };
            await SendRequestAsync(request_body);

            if (status == UpdateRoomStatusNoticeMessage.Status.Remove) {
                isHostingRoom = false;
            }
        }

        /// <summary>
        /// Remove hosting room from the server.
        /// </summary>
        /// <exception cref="ClientErrorException"></exception>
        /// <returns></returns>
        public async Task RemoveHostingRoom() {
            await UpdateHostingRoomStatus(UpdateRoomStatusNoticeMessage.Status.Remove);
        }

        private readonly TcpClient _tcpClient = new TcpClient();
        private uint _sessionKey;
        private byte _hostingRoomGroupIndex;
        private uint _hostingRoomId;

        /// <summary>
        /// Send a request or notice message to the server.
        /// </summary>
        /// <typeparam name="T">A type of the message</typeparam>
        /// <param name="message_body"></param>
        /// <exception cref="ClientErrorException"></exception>
        /// <returns></returns>
        private async Task SendRequestAsync<T>(T message_body) {
            try {
                await _tcpClient.SendRequestMessage(message_body, _sessionKey);
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
                var (error_code, reply_body) = await _tcpClient.ReceiveReplyMessage<T>();
                if (error_code != MessageErrorCode.Ok) {
                    throw new ClientErrorException(ClientErrorCode.RequestError, error_code.ToString());
                }

                return reply_body;
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
            isHostingRoom = false;
        }
    }
}