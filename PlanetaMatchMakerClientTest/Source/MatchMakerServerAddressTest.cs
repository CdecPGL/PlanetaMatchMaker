using System;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace PlanetaGameLabo.MatchMaker.Test
{
    [TestClass]
    [TestCategory("MatchMakerServerAddress")]
    public class MatchMakerServerAddressTest
    {
        [DataTestMethod]
        [DataRow("127.0.0.1")]
        [DataRow("2001:db8::9abc")]
        [DataRow("match.example.com")]
        [DataRow("localhost")]
        public void ConstructorAcceptsValidAddress(string value)
        {
            var address = new MatchMakerServerAddress(value);

            Assert.AreEqual(value, address.Value);
            Assert.AreEqual(value, address.ToString());
        }

        [DataTestMethod]
        [DataRow("2001::db8::9abc")]
        [DataRow(".example.com")]
        [DataRow("example.com.")]
        [DataRow("example..com")]
        [DataRow("")]
        public void ConstructorRejectsInvalidAddress(string value)
        {
            Assert.ThrowsException<ArgumentException>(() => new MatchMakerServerAddress(value));
        }

        [TestMethod]
        public void ConstructorRejectsNullAddress()
        {
            Assert.ThrowsException<ArgumentNullException>(() => new MatchMakerServerAddress(null));
        }

        [TestMethod]
        public void TryParseReturnsAddressForValidAddress()
        {
            Assert.IsTrue(MatchMakerServerAddress.TryParse("match.example.com", out var address));
            Assert.AreEqual("match.example.com", address.Value);
        }

        [TestMethod]
        public void TryParseReturnsFalseForInvalidAddress()
        {
            Assert.IsFalse(MatchMakerServerAddress.TryParse("example..com", out var address));
            Assert.IsNull(address);
        }

        [TestMethod]
        public void EqualsIgnoresHostNameCase()
        {
            Assert.AreEqual(
                new MatchMakerServerAddress("MATCH.example.com"),
                new MatchMakerServerAddress("match.example.com"));
        }
    }
}
