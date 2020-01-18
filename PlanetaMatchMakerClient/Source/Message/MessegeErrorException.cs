using System;

namespace PlanetaGameLabo.MatchMaker
{
#pragma warning disable CA1032
    /// <summary>
    /// An exception of message send and receive error.
    /// </summary>
    public sealed class MessageErrorException : Exception
    {
        public MessageErrorException(string message) : base(message)
        {
        }
    }
#pragma warning restore CA1032
}