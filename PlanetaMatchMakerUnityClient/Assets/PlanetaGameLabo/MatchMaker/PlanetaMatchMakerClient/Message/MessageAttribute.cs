using System;

namespace PlanetaGameLabo.MatchMaker
{
    [AttributeUsage(AttributeTargets.Struct)]
    internal sealed class MessageAttribute : Attribute
    {
        public MessageAttribute(MessageType messageType)
        {
            this.MessageType = messageType;
        }

        public MessageType MessageType { get; }
    }
}