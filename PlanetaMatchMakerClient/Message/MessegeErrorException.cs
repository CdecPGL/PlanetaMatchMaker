using System;

namespace PlanetaGameLabo.MatchMaker
{
    /// <summary>
    /// An exception of message send and receive error in system.
    /// </summary>
    public sealed class MessageInternalErrorException : Exception
    {
        public MessageInternalErrorException(string message) : base(message)
        {
        }
    }
}