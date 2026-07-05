using System;
using System.Linq;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace PlanetaGameLabo.MatchMaker.Test
{
    [TestClass]
    [TestCategory("MatchMakerValueObjects")]
    public class MatchMakerValueObjectsTest
    {
        [TestMethod]
        public void GameIdRejectsEmptyId()
        {
            Assert.AreEqual("game", new MatchMakerGameId("game").Value);
            Assert.IsFalse(MatchMakerGameId.TryParse("", out _));
        }

        [TestMethod]
        public void GameVersionAcceptsEmptyVersion()
        {
            Assert.AreEqual("", MatchMakerGameVersion.Empty.Value);
            Assert.AreEqual("1.0.0", new MatchMakerGameVersion("1.0.0").Value);
        }

        [TestMethod]
        public void AddressAcceptsHostOrIp()
        {
            Assert.AreEqual("match.example.com", new MatchMakerAddress("match.example.com").Value);
            Assert.AreEqual("127.0.0.1", new MatchMakerAddress("127.0.0.1").Value);
            Assert.AreEqual("2001:db8::9abc", new MatchMakerAddress("2001:db8::9abc").Value);
        }

        [TestMethod]
        public void AddressRejectsInvalidHost()
        {
            Assert.IsFalse(MatchMakerAddress.TryParse("example..com", out var address));
            Assert.IsNull(address);
        }

        [TestMethod]
        public void ServerPortRejectsZero()
        {
            Assert.IsTrue(MatchMakerServerPort.TryParse(1, out var port));
            Assert.AreEqual((ushort)1, port.Value);
            Assert.IsFalse(MatchMakerServerPort.TryParse(0, out _));
        }

        [TestMethod]
        public void PlayerNameRejectsEmptyName()
        {
            Assert.AreEqual("player", new MatchMakerPlayerName("player").Value);
            Assert.IsFalse(MatchMakerPlayerName.TryParse("", out _));
        }

        [TestMethod]
        public void GameHostPortAcceptsDynamicPrivatePortOnly()
        {
            Assert.IsFalse(MatchMakerGameHostPort.TryParse(49151, out _));
            Assert.IsTrue(MatchMakerGameHostPort.TryParse(49152, out var port));
            Assert.AreEqual((ushort)49152, port.Value);
        }

        [TestMethod]
        public void ExternalIdCopiesInputArray()
        {
            var source = new byte[] { 1, 2, 3 };
            var externalId = new MatchMakerGameHostExternalId(source);

            source[0] = 9;
            var copied = externalId.ToArray();
            copied[1] = 9;

            CollectionAssert.AreEqual(new byte[] { 1, 2, 3 }, externalId.ToArray());
        }

        [TestMethod]
        public void ExternalIdRejectsTooLongArray()
        {
            var tooLong = Enumerable.Repeat<byte>(1, RoomConstants.GameHostExternalIdLength + 1).ToArray();

            Assert.IsFalse(MatchMakerGameHostExternalId.TryParse(tooLong, out _));
        }

        [TestMethod]
        public void RoomPasswordAcceptsEmptyPassword()
        {
            Assert.AreEqual("", MatchMakerRoomPassword.Empty.Value);
            Assert.AreEqual("secret", new MatchMakerRoomPassword("secret").Value);
        }

        [TestMethod]
        public void SearchNameAcceptsEmptyName()
        {
            Assert.AreEqual("", MatchMakerSearchName.Empty.Value);
            Assert.AreEqual("player", new MatchMakerSearchName("player").Value);
        }

        [TestMethod]
        public void SearchNameRejectsNullName()
        {
            Assert.ThrowsException<ArgumentNullException>(() => new MatchMakerSearchName(null));
        }
    }
}
