using System;
using System.Linq;
using System.Text;

namespace PlanetaGameLabo.MatchMaker
{
    public sealed class AuthenticationOptions
    {
        private AuthenticationOptions(AuthenticationMethod method, byte[] credential)
        {
            if (credential == null)
            {
                throw new ArgumentNullException(nameof(credential));
            }

            if (credential.Length == 0)
            {
                throw new ArgumentException("Credential must not be empty.", nameof(credential));
            }

            if (credential.Length > ClientConstants.MaxMessageAttachmentLength)
            {
                throw new ArgumentOutOfRangeException(nameof(credential),
                    $"Credential must be at most {ClientConstants.MaxMessageAttachmentLength} bytes.");
            }

            Method = method;
            Credential = credential.ToArray();
        }

        public AuthenticationMethod Method { get; }

        internal byte[] Credential { get; }

        public static AuthenticationOptions Steam(byte[] ticket)
        {
            return new AuthenticationOptions(AuthenticationMethod.Steam, ticket);
        }

        public static AuthenticationOptions Oidc(string token)
        {
            if (token == null)
            {
                throw new ArgumentNullException(nameof(token));
            }

            return new AuthenticationOptions(AuthenticationMethod.Oidc, Encoding.UTF8.GetBytes(token));
        }
    }
}
