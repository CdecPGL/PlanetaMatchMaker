using System;
using CdecPGL.MinimalSerializer;

namespace PlanetaGameLabo.MatchMaker
{
    [Serializable]
    public struct PlayerFullName
    {
        [FixedLength(ClientConstants.PlayerNameLength)] public string Name;

        public ushort Tag;

        public string GenerateFullName()
        {
            return $"{Name}#{Tag}";
        }
    }
}