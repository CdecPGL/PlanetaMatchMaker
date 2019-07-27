using System;
using System.Collections.Generic;
using System.IO;
using System.Net.Sockets;
using System.Runtime.Serialization;

namespace PlanetaGameLabo {
    public sealed class MatchMakerClient {
        public async void StartAsync(string server_address, short port) {
            _tcpClient = new TcpClient();
            await _tcpClient.ConnectAsync(server_address, port);

            var header = new RequestMessageHeader {
                messageType = MessageType.AuthenticationRequest
            };
            var request_header_data = new ArraySegment<byte>(Serializer.Serialize(header));
            var message = new AuthenticationRequestMessage {version = Constants.clientVersion};
            var request_body_data = new ArraySegment<byte>(Serializer.Serialize(message));
            await _tcpClient.Client.SendAsync(new ArraySegment<byte>[] {request_header_data, request_body_data},
                SocketFlags.None);

            var reply_header_data =
                new ArraySegment<byte>(new byte[Serializer.GetSerializedSize<ReplyMessageHeader>()]);
            await _tcpClient.Client.ReceiveAsync(reply_header_data, SocketFlags.None);
            var reply_header = Serializer.Deserialize<ReplyMessageHeader>(reply_header_data.Array);
            if (reply_header.messageType != MessageType.AuthenticationReply) {

            }

            var reply_body_data =
                new ArraySegment<byte>(new byte[Serializer.GetSerializedSize<AuthenticationReplyMessage>()]);
            await _tcpClient.Client.ReceiveAsync(reply_body_data, SocketFlags.None);
            var reply_body = Serializer.Deserialize<AuthenticationReplyMessage>(reply_body_data.Array);
            _sessionKey = reply_body.sessionKey;
        }

        public async void GetRoomGroupListAsync() {
            var data = new byte[0];
            await _tcpClient.Client.SendAsync(new ArraySegment<byte>(data), SocketFlags.None);
            var buffer = new byte[0];
            await _tcpClient.Client.ReceiveAsync(new ArraySegment<byte>(buffer), SocketFlags.None);
        }

        public async void CreateRoomAsync(int room_group_index, string room_name) {
            var data = new byte[0];
            await _tcpClient.Client.SendAsync(new ArraySegment<byte>(data), SocketFlags.None);
            var buffer = new byte[0];
            await _tcpClient.Client.ReceiveAsync(new ArraySegment<byte>(buffer), SocketFlags.None);
        }

        public async void GetRoomList(int room_group_index) {
            var data = new byte[0];
            await _tcpClient.Client.SendAsync(new ArraySegment<byte>(data), SocketFlags.None);
            var buffer = new byte[0];
            await _tcpClient.Client.ReceiveAsync(new ArraySegment<byte>(buffer), SocketFlags.None);
        }

        public async void JoinRoom(int room_group_index, int room_id) {
            var data = new byte[0];
            await _tcpClient.Client.SendAsync(new ArraySegment<byte>(data), SocketFlags.None);
            var buffer = new byte[0];
            await _tcpClient.Client.ReceiveAsync(new ArraySegment<byte>(buffer), SocketFlags.None);
        }

        public async void UpdateRoomStatus(int room_group_index, int room_id) {
            var data = new byte[0];
            await _tcpClient.Client.SendAsync(new ArraySegment<byte>(data), SocketFlags.None);
        }

        private TcpClient _tcpClient;
        private uint _sessionKey;
    }
}