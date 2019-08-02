using System;

namespace PlanetaGameLabo.MatchMaker {
    public sealed class MessageErrorException : Exception {
        public MessageErrorException(string message) : base(message) { }
    }
}