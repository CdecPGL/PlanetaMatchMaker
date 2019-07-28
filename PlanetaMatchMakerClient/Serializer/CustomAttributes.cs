using System;
using System.Xml.Schema;

namespace PlanetaGameLabo.Serializer {
    [AttributeUsage(AttributeTargets.Field)]
    public sealed class FixedLengthAttribute : Attribute {
        public int length { get; }

        public FixedLengthAttribute(int length) {
            this.length = length;
        }
    }
}