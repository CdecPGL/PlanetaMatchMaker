using System;
using System.Collections.Generic;
using System.Text;

namespace PlanetaGameLabo.MatchMaker {
    [AttributeUsage(AttributeTargets.Struct)]
    public sealed class MessageAttribute : Attribute {
        public MessageAttribute(MessageType message_type) {
            messageType = message_type;
        }

        public MessageType messageType { get; }
    }
}