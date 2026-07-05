using System;
using System.Linq;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace PlanetaGameLabo.MatchMaker.Test
{
    [TestClass]
    [TestCategory("ValueObjects")]
    public class ValueObjectsTest
    {
        [TestMethod]
        public void GameIdRejectsEmptyId()
        {
            Assert.AreEqual("game", new GameId("game").Value);
            Assert.IsFalse(GameId.TryParse("", out _));
        }

        [TestMethod]
        public void GameVersionAcceptsEmptyVersion()
        {
            Assert.AreEqual("", GameVersion.Empty.Value);
            Assert.AreEqual("1.0.0", new GameVersion("1.0.0").Value);
        }

        [TestMethod]
        public void AddressAcceptsHostOrIp()
        {
            Assert.AreEqual("match.example.com", new NetworkServerAddress("match.example.com").Value);
            Assert.AreEqual("127.0.0.1", new NetworkServerAddress("127.0.0.1").Value);
            Assert.AreEqual("2001:db8::9abc", new NetworkServerAddress("2001:db8::9abc").Value);
        }

        [TestMethod]
        public void AddressRejectsInvalidHost()
        {
            Assert.IsFalse(NetworkServerAddress.TryParse("example..com", out var address));
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
            Assert.AreEqual("player", new PlayerName("player").Value);
            Assert.IsFalse(PlayerName.TryParse("", out _));
        }

        [TestMethod]
        public void GameHostPortAcceptsDynamicPrivatePortOnly()
        {
            Assert.IsFalse(GameHostPort.TryParse(49151, out _));
            Assert.IsTrue(GameHostPort.TryParse(49152, out var port));
            Assert.AreEqual((ushort)49152, port.Value);
        }

        [TestMethod]
        public void ExternalIdCopiesInputArray()
        {
            var source = new byte[] { 1, 2, 3 };
            var externalId = new GameHostExternalId(source);

            source[0] = 9;
            var copied = externalId.ToArray();
            copied[1] = 9;

            CollectionAssert.AreEqual(new byte[] { 1, 2, 3 }, externalId.ToArray());
        }

        [TestMethod]
        public void ExternalIdRejectsTooLongArray()
        {
            var tooLong = Enumerable.Repeat<byte>(1, RoomConstants.GameHostExternalIdLength + 1).ToArray();

            Assert.IsFalse(GameHostExternalId.TryParse(tooLong, out _));
        }

        [TestMethod]
        public void RoomPasswordAcceptsEmptyPassword()
        {
            Assert.AreEqual("", RoomPassword.Empty.Value);
            Assert.AreEqual("secret", new RoomPassword("secret").Value);
        }

        [TestMethod]
        public void SearchNameAcceptsEmptyName()
        {
            Assert.AreEqual("", SearchName.Empty.Value);
            Assert.AreEqual("player", new SearchName("player").Value);
        }

        [TestMethod]
        public void SearchNameRejectsNullName()
        {
            Assert.ThrowsException<ArgumentNullException>(() => new SearchName(null));
        }
    }
}
