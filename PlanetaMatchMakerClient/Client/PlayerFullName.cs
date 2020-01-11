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
    }
}