using System;

namespace PlanetaGameLabo {
    public class InvalidSerializationException : Exception {
        public InvalidSerializationException() : base() { }
        public InvalidSerializationException(string message) : base(message) { }
    }
}