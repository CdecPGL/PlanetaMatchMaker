namespace PlanetaGameLabo.MatchMaker
{
    /// <summary>
    /// Immutable PMMS server address. Use <see cref="MatchMakerAddress"/> for new host or IP address APIs.
    /// </summary>
    public sealed class MatchMakerServerAddress : MatchMakerAddress
    {
        public MatchMakerServerAddress(string value) : base(value)
        {
        }

        public new static MatchMakerServerAddress Parse(string value)
        {
            return new MatchMakerServerAddress(value);
        }

        public static bool TryParse(string value, out MatchMakerServerAddress address)
        {
            if (!MatchMakerAddress.TryParse(value, out var parsedAddress))
            {
                address = null;
                return false;
            }

            address = new MatchMakerServerAddress(parsedAddress.Value);
            return true;
        }
    }
}
