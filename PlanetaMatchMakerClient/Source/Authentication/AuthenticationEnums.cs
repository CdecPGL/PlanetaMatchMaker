namespace PlanetaGameLabo.MatchMaker
{
#pragma warning disable CA1028
    public enum AuthenticationMethod : byte
    {
        None = 0,
        Steam = 1,
    }

    public enum AuthenticationResult : byte
    {
        Success,
        ApiVersionMismatch,
        GameIdMismatch,
        GameVersionMismatch,
        UnsupportedAuthenticationMethod,
        AuthenticationDataFormatInvalid,
        AuthenticationDataSizeExceeded,
        AuthenticationDataInvalid,
        InsecureConnection,
        SteamTicketInvalid,
        SteamIdMismatch,
        SteamOwnershipCheckFailed,
        SteamAuthenticationServiceUnavailable,
    }
#pragma warning restore CA1028
}
