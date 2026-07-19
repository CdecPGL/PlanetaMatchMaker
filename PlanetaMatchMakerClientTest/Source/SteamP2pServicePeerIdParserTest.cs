using Microsoft.VisualStudio.TestTools.UnitTesting;
using PlanetaGameLabo.MatchMaker.Extentions;

namespace PlanetaGameLabo.MatchMaker.Test
{
    [TestClass]
    public class SteamP2pServicePeerIdParserTest
    {
        [TestMethod]
        public void CanonicalDecimalSteamId64IsAccepted()
        {
            Assert.IsTrue(SteamP2pServicePeerIdParser.TryParse(
                new P2pServicePeerId("76561198000000000"), out var steamId64));
            Assert.AreEqual(76561198000000000ul, steamId64);
        }

        [DataTestMethod]
        [DataRow("")]
        [DataRow("+76561198000000000")]
        [DataRow(" 76561198000000000")]
        [DataRow("76561198000000000 ")]
        [DataRow("steam-user")]
        [DataRow("18446744073709551616")]
        public void NonCanonicalOrOutOfRangeSteamId64IsRejected(string value)
        {
            Assert.IsFalse(SteamP2pServicePeerIdParser.TryParse(new P2pServicePeerId(value), out _));
        }
    }
}
