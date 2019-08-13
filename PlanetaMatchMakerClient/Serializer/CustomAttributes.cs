using System;
using System.Xml.Schema;

namespace PlanetaGameLabo.Serializer {
    [AttributeUsage(AttributeTargets.Field)]
    public sealed class FixedLengthAttribute : Attribute {
        public int Length { get; }

        public FixedLengthAttribute(int length) {
            Length = length;
        }
    }
}