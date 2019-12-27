using System;

namespace PlanetaGameLabo.MatchMaker
{
    /// <summary>
    /// An exception of message send and receive error in system.
    /// </summary>
    public sealed class MessageErrorException : Exception
    {
        public MessageErrorException(string message) : base(message)
        {
        }
    }
}