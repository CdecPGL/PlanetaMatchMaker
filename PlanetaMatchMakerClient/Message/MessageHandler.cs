using System;
using System.Collections.Generic;
using System.Net.Sockets;
using System.Reflection;
using System.Threading.Tasks;

namespace PlanetaGameLabo.MatchMaker {
    internal static class MessageHandler {
        internal static async Task SendRequestMessage<T>(TcpClient client, T message_body, uint sessionKey = 0) {
            var message_attribute = message_body.GetType().GetCustomAttribute<MessageAttribute>();
            if (message_attribute == null) {
                throw null;
            }

            var header = new RequestMessageHeader {
                messageType = message_attribute.messageType,
                sessionKey = sessionKey
            };
            var request_header_data = new ArraySegment<byte>(Serializer.Serializer.Serialize(header));
            var request_body_data = new ArraySegment<byte>(Serializer.Serializer.Serialize(message_body));
            var data = new List<ArraySegment<byte>> {request_header_data, request_body_data};
            await client.Client.SendAsync(data, SocketFlags.None);
        }

        internal static async Task<(MessageErrorCode, T)> ReceiveReplyMessage<T>(TcpClient client) {
            var message_attribute = typeof(T).GetCustomAttribute<MessageAttribute>();
            if (message_attribute == null) {
                throw null;
            }

            var buffer =
                new ArraySegment<byte>(new byte[Serializer.Serializer.GetSerializedSize<ReplyMessageHeader>()]);
            await client.Client.ReceiveAsync(buffer, SocketFlags.None);
            var header = Serializer.Serializer.Deserialize<ReplyMessageHeader>(buffer.Array);
            if (header.errorCode != MessageErrorCode.Ok) {
                return (header.errorCode, default);
            }

            if (header.messageType != message_attribute.messageType) {
                throw null;
            }

            buffer = new ArraySegment<byte>(new byte[Serializer.Serializer.GetSerializedSize<T>()]);
            await client.Client.ReceiveAsync(buffer, SocketFlags.None);
            var body = Serializer.Serializer.Deserialize<T>(buffer.Array);
            return (MessageErrorCode.Ok, body);
        }
    }
}