using System;

namespace PlanetaGameLabo.Serializer {
    public sealed class InvalidSerializationException : Exception {
        public InvalidSerializationException() : base() { }
        public InvalidSerializationException(string message) : base(message) { }
    }
}