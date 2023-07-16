using System;

namespace PlanetaGameLabo.MatchMaker
{
#pragma warning disable CA1032
    /// <summary>
    /// An exception of authentication error.
    /// </summary>
    public sealed class AuthenticationErrorException : Exception
    {
        /// <summary>
        /// An error code of authentication.
        /// </summary>
        public AuthenticationErrorCode AuthenticationErrorCode { get; }

        /// <summary>
        /// A API version of the server.
        /// </summary>
        public ushort ServerApiVersion { get; }

        /// <summary>
        /// A game version the server acceptable.
        /// </summary>
        public string ServerGameVersion { get; }


        public AuthenticationErrorException(AuthenticationErrorCode errorCode, ushort serverApiVersion,
            string serverGameVersion) : base(
            $"Authentication error ({errorCode})")
        {
            AuthenticationErrorCode = errorCode;
            ServerApiVersion = serverApiVersion;
            ServerGameVersion = serverGameVersion;
        }
    }

#pragma warning restore CA1032

    public enum AuthenticationErrorCode
    {
        ApiVersionMismatch,
        GameIdMismatch,
        GameVersionMismatch,
    };

    public static class AuthenticationErrorCodeExtension
    {
        public static AuthenticationErrorCode ToErrorCode(this AuthenticationResult result)
        {
            switch (result)
            {
                case AuthenticationResult.Success:
                    throw new ArgumentOutOfRangeException(nameof(result), result, null);
                case AuthenticationResult.ApiVersionMismatch:
                    return AuthenticationErrorCode.ApiVersionMismatch;
                case AuthenticationResult.GameIdMismatch:
                    return AuthenticationErrorCode.GameIdMismatch;
                case AuthenticationResult.GameVersionMismatch:
                    return AuthenticationErrorCode.GameVersionMismatch;
                default:
                    throw new ArgumentOutOfRangeException(nameof(result), result, null);
            }
        }
    }
}
