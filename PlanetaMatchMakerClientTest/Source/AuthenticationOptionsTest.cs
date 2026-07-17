using System;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace PlanetaGameLabo.MatchMaker.Tests
{
    [TestClass]
    public class AuthenticationOptionsTest
    {
        [TestMethod]
        public void NoneUsesEmptyCredential()
        {
            var options = AuthenticationOptions.None();

            Assert.AreEqual(AuthenticationMethod.None, options.Method);
            Assert.AreEqual(0, options.Credential.Length);
        }

        [TestMethod]
        public void SteamRejectsCredentialOverProtocolMaximum()
        {
            var credential = new byte[ClientConstants.MaxMessageAttachmentLength + 1];

            Assert.ThrowsException<ArgumentOutOfRangeException>(() => AuthenticationOptions.Steam(credential));
        }

        [TestMethod]
        public void SteamAcceptsCredentialAtProtocolMaximum()
        {
            var credential = new byte[ClientConstants.MaxMessageAttachmentLength];

            var options = AuthenticationOptions.Steam(credential);

            Assert.AreEqual(AuthenticationMethod.Steam, options.Method);
        }
    }
}
