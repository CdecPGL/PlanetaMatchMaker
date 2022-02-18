using System;
using CdecPGL.MinimalSerializer;

namespace PlanetaGameLabo.MatchMaker
{
    [Serializable]
    public struct PlayerFullName : IEquatable<PlayerFullName>
    {
        [FixedLength(ClientConstants.PlayerNameLength)]
        public string Name;

        public ushort Tag;

        public const ushort NotAssignedTag = 0;

        public string GenerateFullName()
        {
            return $"{Name}#{Tag}";
        }

        public override string ToString()
        {
            return GenerateFullName();
        }

        public override bool Equals(object obj)
        {
            if (obj == null)
            {
                return false;
            }

            try
            {
                var other = (PlayerFullName)obj;
                return Equals(other);
            }
            catch (InvalidCastException)
            {
                return false;
            }
        }

        public static bool operator ==(PlayerFullName left, PlayerFullName right)
        {
            return left.Equals(right);
        }

        public static bool operator !=(PlayerFullName left, PlayerFullName right)
        {
            return !(left == right);
        }

        public bool Equals(PlayerFullName other)
        {
            return Name == other.Name && Tag == other.Tag;
        }

        public override int GetHashCode()
        {
            return new { Name, Tag }.GetHashCode();
        }
    }
}