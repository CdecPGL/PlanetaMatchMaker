using System;
using System.Xml.Schema;

namespace PlanetaMatchMakerClient {
    [AttributeUsage(AttributeTargets.Field)]
    public sealed class FixedLengthAttribute : Attribute {
        public int length { get; }

        public FixedLengthAttribute(int length) {
            this.length = length;
        }
    }
}