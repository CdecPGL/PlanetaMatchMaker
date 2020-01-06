using System;

namespace PlanetaGameLabo.MatchMaker
{
#pragma warning disable CA1032
    /// <summary>
    /// An exception of message send and receive error in system.
    /// </summary>
    public sealed class MessageInternalErrorException : Exception
    {
        public MessageInternalErrorException(string message) : base(message)
        {
        }
    }
#pragma warning restore CA1032
}