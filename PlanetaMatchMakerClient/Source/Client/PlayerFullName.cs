using System;
using CdecPGL.MinimalSerializer;

namespace PlanetaGameLabo.MatchMaker
{
    [Serializable]
    public struct PlayerFullName
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
                return Name == other.Name && Tag == other.Tag;
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
    }
}