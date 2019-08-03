using System.Linq;
using System.Net.Sockets;
using System.Threading.Tasks;

namespace PlanetaGameLabo.MatchMaker {
    public sealed class MatchMakerClient {
        public async Task ConnectAsync(string server_address, ushort port) {
            _tcpClient = new TcpClient();
            await _tcpClient.ConnectAsync(server_address, port);

            var request_body = new AuthenticationRequestMessage {version = ClientConstants.clientVersion};
            await SendRequestAsync(request_body);

            var reply_body = await ReceiveReplyAsync<AuthenticationReplyMessage>();
            _sessionKey = reply_body.sessionKey;
        }

        public void Close() {
            _tcpClient.Close();
        }

        public async Task<ListRoomGroupReplyMessage.RoomGroupInfo[]> GetRoomGroupListAsync() {
            var request_body = new ListRoomGroupRequestMessage();
            await SendRequestAsync(request_body);

            var reply_body = await ReceiveReplyAsync<ListRoomGroupReplyMessage>();
            return reply_body.roomGroupInfoList.Take(reply_body.roomGroupCount).ToArray();
        }

        public async Task CreateRoomAsync(byte room_group_index, string room_name) {
            var request_body = new CreateRoomRequestMessage {
                groupIndex = room_group_index,
                name = room_name
            };
            await SendRequestAsync(request_body);

            await ReceiveReplyAsync<CreateRoomReplyMessage>();
        }

        public async Task<(int totalRoomCount, ListRoomReplyMessage.RoomInfo[] roomInfoList)> GetRoomList(
            byte room_group_index, byte start_index,
            byte count, RoomDataSortKind sort_kind, byte flags) {
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

        public async Task<ClientAddress> JoinRoom(byte room_group_index, uint room_id) {
            var request_body = new JoinRoomRequestMessage {
                groupIndex = room_group_index,
                roomId = room_id
            };
            await SendRequestAsync(request_body);

            var reply_body = await ReceiveReplyAsync<JoinRoomReplyMessage>();
            return reply_body.hostAddress;
        }

        public async Task UpdateRoomStatus(byte room_group_index, uint room_id,
            UpdateRoomStatusNoticeMessage.Status status) {
            var request_body = new UpdateRoomStatusNoticeMessage {
                groupIndex = room_group_index,
                roomId = room_id,
                status = status
            };
            await SendRequestAsync(request_body);
        }

        private TcpClient _tcpClient;
        private uint _sessionKey;

        private async Task SendRequestAsync<T>(T message_body) {
            try {
                await MessageUtilities.SendRequestMessage(_tcpClient, message_body, _sessionKey);
            }
            catch (MessageErrorException e) {
                throw new ClientErrorException(ClientErrorCode.MessageSendError, e.Message);
            }
        }

        private async Task<T> ReceiveReplyAsync<T>() {
            try {
                var (error_code, reply_body) = await MessageUtilities.ReceiveReplyMessage<T>(_tcpClient);
                if (error_code != MessageErrorCode.Ok) {
                    throw new ClientErrorException(ClientErrorCode.RequestError, error_code.ToString());
                }

                return reply_body;
            }
            catch (MessageErrorException e) {
                throw new ClientErrorException(ClientErrorCode.MessageReceptionError, e.Message);
            }
        }
    }
}