namespace PlanetaGameLabo.MatchMaker
{
    public enum AuthenticationResult : byte
    {
        Success,
        ApiVersionMismatch,
        GameIdMismatch,
        GameVersionMismatch,
    }
}
