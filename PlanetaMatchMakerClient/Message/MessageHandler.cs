using System;
using System.Collections.Generic;
using System.Text;

namespace PlanetaGameLabo.MatchMaker {
    public static class MessageHandler {
        public static ArraySegment<byte>[] PackRequestMessage<T>(in T body) {
            var header = new RequestMessageHeader {
                messageType = MessageType.AuthenticationRequest
            };
            var request_header_data = new ArraySegment<byte>(Serializer.Serializer.Serialize(header));
            var message = new AuthenticationRequestMessage {version = ClientConstants.clientVersion};
            var request_body_data = new ArraySegment<byte>(Serializer.Serializer.Serialize(message));
            return new[] {request_header_data, request_body_data};
        }
    }
}