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
        public void ValueObjectsAreValueTypes()
        {
            Assert.IsTrue(typeof(GameId).IsValueType);
            Assert.IsTrue(typeof(GameVersion).IsValueType);
            Assert.IsTrue(typeof(Host).IsValueType);
            Assert.IsTrue(typeof(ServerPort).IsValueType);
            Assert.IsTrue(typeof(PlayerName).IsValueType);
            Assert.IsTrue(typeof(GameHostPort).IsValueType);
            Assert.IsTrue(typeof(GameHostExternalId).IsValueType);
            Assert.IsTrue(typeof(RoomPassword).IsValueType);
            Assert.IsTrue(typeof(SearchName).IsValueType);
        }

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
        public void HostAcceptsHostNameOrIp()
        {
            Assert.AreEqual("match.example.com", new Host("match.example.com").Value);
            Assert.AreEqual("127.0.0.1", new Host("127.0.0.1").Value);
            Assert.AreEqual("2001:db8::9abc", new Host("2001:db8::9abc").Value);
        }

        [DataTestMethod]
        [DataRow("127.0.0.1")]
        [DataRow("0.0.0.0")]
        [DataRow("255.255.255.255")]
        [DataRow("2001:0db8:bd05:01d2:288a:1fc0:0001:10ee")]
        [DataRow("2001:db8::9abc")]
        [DataRow("a.b.c")]
        [DataRow("a")]
        public void HostTryParseAcceptsValidValues(string value)
        {
            Assert.IsTrue(Host.TryParse(value, out var host));
            Assert.AreEqual(value, host.Value);
        }

        [TestMethod]
        public void HostRejectsInvalidHostName()
        {
            Assert.ThrowsException<ArgumentException>(() => new Host("example..com"));
            Assert.IsFalse(Host.TryParse("example..com", out var host));
            Assert.AreEqual(default(Host), host);
        }

        [DataTestMethod]
        [DataRow("2001:0db8:bd05:01d2:288a:1fc0:0001:10ee:ffff")]
        [DataRow("2001::db8::9abc")]
        [DataRow(".a.b.c")]
        [DataRow("a.b.c.")]
        [DataRow("a.b..c")]
        [DataRow(".")]
        [DataRow("")]
        [DataRow(null)]
        public void HostTryParseRejectsInvalidValues(string value)
        {
            Assert.IsFalse(Host.TryParse(value, out var host));
            Assert.AreEqual(default(Host), host);
        }

        [TestMethod]
        public void ServerPortRejectsZero()
        {
            Assert.IsTrue(ServerPort.TryParse(1, out var port));
            Assert.AreEqual((ushort)1, port.Value);
            Assert.IsFalse(ServerPort.TryParse(0, out _));
        }

        [DataTestMethod]
        [DataRow((ushort)1)]
        [DataRow((ushort)65535)]
        public void ServerPortAcceptsValidValues(ushort value)
        {
            Assert.IsTrue(ServerPort.TryParse(value, out var port));
            Assert.AreEqual(value, port.Value);
        }

        [TestMethod]
        public void PlayerNameRejectsEmptyName()
        {
            Assert.AreEqual("player", new PlayerName("player").Value);
            Assert.IsFalse(PlayerName.TryParse("", out _));
        }

        [DataTestMethod]
        [DataRow("p")]
        [DataRow("123456789012345678901234")]
        [DataRow("ああああああああ")]
        [DataRow("あああああああaaa")]
        public void PlayerNameAcceptsValidValues(string value)
        {
            Assert.IsTrue(PlayerName.TryParse(value, out var playerName));
            Assert.AreEqual(value, playerName.Value);
        }

        [DataTestMethod]
        [DataRow(null)]
        [DataRow("")]
        [DataRow("1234567890123456789012345")]
        [DataRow("あああああああああ")]
        [DataRow("ああああああああaaa")]
        [DataRow("あああああああaaaa")]
        public void PlayerNameRejectsInvalidValues(string value)
        {
            Assert.IsFalse(PlayerName.TryParse(value, out var playerName));
            Assert.AreEqual(default(PlayerName), playerName);
        }

        [TestMethod]
        public void GameHostPortAcceptsDynamicPrivatePortOnly()
        {
            Assert.IsFalse(GameHostPort.TryParse(49151, out _));
            Assert.IsTrue(GameHostPort.TryParse(49152, out var port));
            Assert.AreEqual((ushort)49152, port.Value);
        }

        [DataTestMethod]
        [DataRow((ushort)49152)]
        [DataRow((ushort)65535)]
        public void GameHostPortAcceptsValidValues(ushort value)
        {
            Assert.IsTrue(GameHostPort.TryParse(value, out var port));
            Assert.AreEqual(value, port.Value);
        }

        [DataTestMethod]
        [DataRow((ushort)0)]
        [DataRow((ushort)49151)]
        public void GameHostPortRejectsInvalidValues(ushort value)
        {
            Assert.IsFalse(GameHostPort.TryParse(value, out var port));
            Assert.AreEqual(default(GameHostPort), port);
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
        public void ExternalIdAcceptsMaxLengthArray()
        {
            var maxLength = Enumerable.Repeat<byte>(1, RoomConstants.GameHostExternalIdLength).ToArray();

            Assert.IsTrue(GameHostExternalId.TryParse(maxLength, out var externalId));
            CollectionAssert.AreEqual(maxLength, externalId.ToArray());
        }

        [TestMethod]
        public void ExternalIdCreatesFromExplicitSourceTypes()
        {
            var stringExternalId = GameHostExternalId.FromString("external-id");
            var uint64ExternalId = GameHostExternalId.FromUInt64(123456789UL);
            var uint32ExternalId = GameHostExternalId.FromUInt32(123456789U);
            var uint16ExternalId = GameHostExternalId.FromUInt16(12345);

            Assert.IsTrue(stringExternalId.ToArray().Length <= RoomConstants.GameHostExternalIdLength);
            Assert.IsTrue(uint64ExternalId.ToArray().Length <= RoomConstants.GameHostExternalIdLength);
            Assert.IsTrue(uint32ExternalId.ToArray().Length <= RoomConstants.GameHostExternalIdLength);
            Assert.IsTrue(uint16ExternalId.ToArray().Length <= RoomConstants.GameHostExternalIdLength);
        }

        [TestMethod]
        public void RoomPasswordAcceptsEmptyPassword()
        {
            Assert.AreEqual("", RoomPassword.Empty.Value);
            Assert.AreEqual("secret", new RoomPassword("secret").Value);
        }

        [DataTestMethod]
        [DataRow("")]
        [DataRow("p")]
        [DataRow("1234567890123456")]
        [DataRow("あああああ")]
        [DataRow("あああああa")]
        public void RoomPasswordAcceptsValidValues(string value)
        {
            Assert.IsTrue(RoomPassword.TryParse(value, out var password));
            Assert.AreEqual(value, password.Value);
        }

        [DataTestMethod]
        [DataRow(null)]
        [DataRow("1234567890123456789012345")]
        [DataRow("12345678901234567")]
        [DataRow("ああああああ")]
        [DataRow("あああああaa")]
        public void RoomPasswordRejectsInvalidValues(string value)
        {
            Assert.IsFalse(RoomPassword.TryParse(value, out var password));
            Assert.AreEqual(default(RoomPassword), password);
        }

        [TestMethod]
        public void SearchNameAcceptsEmptyName()
        {
            Assert.AreEqual("", SearchName.Empty.Value);
            Assert.AreEqual("player", new SearchName("player").Value);
        }

        [DataTestMethod]
        [DataRow("")]
        [DataRow("p")]
        [DataRow("123456789012345678901234")]
        [DataRow("ああああああああ")]
        [DataRow("あああああああaaa")]
        public void SearchNameAcceptsValidValues(string value)
        {
            Assert.IsTrue(SearchName.TryParse(value, out var searchName));
            Assert.AreEqual(value, searchName.Value);
        }

        [DataTestMethod]
        [DataRow(null)]
        [DataRow("1234567890123456789012345")]
        [DataRow("あああああああああ")]
        [DataRow("ああああああああaaa")]
        [DataRow("あああああああaaaa")]
        public void SearchNameRejectsInvalidValues(string value)
        {
            Assert.IsFalse(SearchName.TryParse(value, out var searchName));
            Assert.AreEqual(default(SearchName), searchName);
        }

        [TestMethod]
        public void SearchNameRejectsNullName()
        {
            Assert.ThrowsException<ArgumentNullException>(() => new SearchName(null));
        }
    }
}
