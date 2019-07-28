using System.Linq;
using System.Net.Sockets;
using System.Threading.Tasks;

namespace PlanetaGameLabo.MatchMaker {
    public sealed class MatchMakerClient {
        public async Task ConnectAsync(string server_address, short port) {
            _tcpClient = new TcpClient();
            await _tcpClient.ConnectAsync(server_address, port);

            var request_body = new AuthenticationRequestMessage {version = ClientConstants.clientVersion};
            await MessageHandler.SendRequestMessage(_tcpClient, request_body, _sessionKey);

            var (error_code, reply_body) =
                await MessageHandler.ReceiveReplyMessage<AuthenticationReplyMessage>(_tcpClient);
            if (error_code != MessageErrorCode.Ok) { }

            _sessionKey = reply_body.sessionKey;
        }

        public void Close() {
            _tcpClient.Close();
        }

        public async Task<ListRoomGroupReplyMessage.RoomGroupInfo[]> GetRoomGroupListAsync() {
            var request_body = new ListRoomGroupRequestMessage();
            await MessageHandler.SendRequestMessage(_tcpClient, request_body, _sessionKey);

            var (error_code, reply_body) =
                await MessageHandler.ReceiveReplyMessage<ListRoomGroupReplyMessage>(_tcpClient);
            if (error_code != MessageErrorCode.Ok) { }

            return reply_body.roomGroupInfoList.Take(reply_body.roomGroupCount).ToArray();
        }

        public async Task CreateRoomAsync(byte room_group_index, string room_name) {
            var request_body = new CreateRoomRequestMessage {
                groupIndex = room_group_index,
                name = room_name
            };
            await MessageHandler.SendRequestMessage(_tcpClient, request_body, _sessionKey);

            var (error_code, reply_body) =
                await MessageHandler.ReceiveReplyMessage<CreateRoomReplyMessage>(_tcpClient);
            if (error_code != MessageErrorCode.Ok) { }
        }

        public async Task<ListRoomReplyMessage.RoomInfo[]> GetRoomList(byte room_group_index, byte start_index,
            byte count, RoomDataSortKind sort_kind, byte flags) {
            var request_body = new ListRoomRequestMessage {
                groupIndex = room_group_index,
                startIndex = start_index,
                endIndex = (byte) (start_index + count - 1),
                sortKind = sort_kind,
                flags = flags
            };
            await MessageHandler.SendRequestMessage(_tcpClient, request_body, _sessionKey);

            var (error_code, reply_body) =
                await MessageHandler.ReceiveReplyMessage<ListRoomReplyMessage>(_tcpClient);
            if (error_code != MessageErrorCode.Ok) { }

            return reply_body.roomInfoList.Take(reply_body.resultRoomCount).ToArray();
        }

        public async Task<ClientAddress> JoinRoom(byte room_group_index, uint room_id) {
            var request_body = new JoinRoomRequestMessage {
                groupIndex = room_group_index,
                roomId = room_id
            };
            await MessageHandler.SendRequestMessage(_tcpClient, request_body, _sessionKey);

            var (error_code, reply_body) =
                await MessageHandler.ReceiveReplyMessage<JoinRoomReplyMessage>(_tcpClient);
            if (error_code != MessageErrorCode.Ok) { }

            return reply_body.hostAddress;
        }

        public async Task UpdateRoomStatus(byte room_group_index, uint room_id,
            UpdateRoomStatusNoticeMessage.Status status) {
            var request_body = new UpdateRoomStatusNoticeMessage {
                groupIndex = room_group_index,
                roomId = room_id,
                status = status
            };
            await MessageHandler.SendRequestMessage(_tcpClient, request_body, _sessionKey);
        }

        private TcpClient _tcpClient;
        private uint _sessionKey;
    }
}