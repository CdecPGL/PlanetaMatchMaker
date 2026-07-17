namespace PlanetaGameLabo.MatchMaker
{
#pragma warning disable CA1028
    public enum AuthenticationMethod : byte
    {
        Steam = 0,
        Oidc = 1,
        None = 2,
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
        OidcTokenInvalid,
        OidcSignatureVerificationFailed,
        OidcIssuerMismatch,
        OidcAudienceMismatch,
        OidcTokenExpired,
        OidcSubjectMissing,
        OidcKeyFetchFailed,
        OidcDisallowedAlgorithm,
    }
#pragma warning restore CA1028
}
