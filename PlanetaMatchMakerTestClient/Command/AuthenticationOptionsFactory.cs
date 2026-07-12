using System;
using System.Linq;

namespace PlanetaGameLabo.MatchMaker
{
    internal enum TestClientAuthenticationMethod
    {
        Oidc,
        Steam,
    }

    internal static class AuthenticationOptionsFactory
    {
        public static AuthenticationOptions Create(TestClientAuthenticationMethod method, string credential)
        {
            if (string.IsNullOrEmpty(credential))
            {
                throw new ArgumentException("Authentication credential is required.", nameof(credential));
            }

            switch (method)
            {
                case TestClientAuthenticationMethod.Oidc:
                    return AuthenticationOptions.Oidc(credential);
                case TestClientAuthenticationMethod.Steam:
                    return AuthenticationOptions.Steam(ParseHex(credential));
                default:
                    throw new ArgumentOutOfRangeException(nameof(method), method, null);
            }
        }

        private static byte[] ParseHex(string value)
        {
            if (value.Length % 2 != 0 || value.Any(c => !Uri.IsHexDigit(c)))
            {
                throw new ArgumentException("Steam credential must be a hex string.", nameof(value));
            }

            var bytes = new byte[value.Length / 2];
            for (var i = 0; i < bytes.Length; ++i)
            {
                bytes[i] = Convert.ToByte(value.Substring(i * 2, 2), 16);
            }

            return bytes;
        }
    }
}
