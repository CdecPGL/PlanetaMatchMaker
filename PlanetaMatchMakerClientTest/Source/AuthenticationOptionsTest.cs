using System;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace PlanetaGameLabo.MatchMaker.Tests
{
    [TestClass]
    public class AuthenticationOptionsTest
    {
        [TestMethod]
        public void SteamRejectsCredentialOverProtocolMaximum()
        {
            var credential = new byte[ClientConstants.MaxAuthenticationCredentialLength + 1];

            Assert.ThrowsException<ArgumentOutOfRangeException>(() => AuthenticationOptions.Steam(credential));
        }

        [TestMethod]
        public void SteamAcceptsCredentialAtProtocolMaximum()
        {
            var credential = new byte[ClientConstants.MaxAuthenticationCredentialLength];

            var options = AuthenticationOptions.Steam(credential);

            Assert.AreEqual(AuthenticationMethod.Steam, options.Method);
        }
    }
}
