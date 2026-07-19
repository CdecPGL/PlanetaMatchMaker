using System;
using System.Globalization;
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
            Assert.IsTrue(typeof(P2pServicePeerId).IsValueType);
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
        public void P2pServicePeerIdStoresUtf8String()
        {
            var peerId = new P2pServicePeerId("peer-123");

            Assert.AreEqual("peer-123", peerId.Value);
            Assert.AreEqual("peer-123", peerId.ToString());
        }

        [TestMethod]
        public void P2pServicePeerIdAccepts128Utf8Bytes()
        {
            var value = new string('x', RoomConstants.P2pServicePeerIdLength);

            Assert.IsTrue(P2pServicePeerId.TryParse(value, out var peerId));
            Assert.AreEqual(value, peerId.Value);
        }

        [TestMethod]
        public void P2pServicePeerIdRejects129Utf8Bytes()
        {
            var value = new string('x', RoomConstants.P2pServicePeerIdLength + 1);

            Assert.IsFalse(P2pServicePeerId.TryParse(value, out _));
            Assert.ThrowsException<ArgumentException>(() => new P2pServicePeerId(value));
        }

        [TestMethod]
        public void P2pServicePeerIdCountsUtf8BytesInsteadOfUtf16Characters()
        {
            Assert.IsTrue(P2pServicePeerId.TryParse(new string('あ', 42), out _));
            Assert.IsFalse(P2pServicePeerId.TryParse(new string('あ', 43), out _));
        }

        [TestMethod]
        public void P2pServicePeerIdRejectsEmbeddedNulAndInvalidUtf16()
        {
            Assert.IsFalse(P2pServicePeerId.TryParse("peer\0suffix", out _));
            Assert.IsFalse(P2pServicePeerId.TryParse("\ud800", out _));
        }

        [TestMethod]
        public void SteamP2pServicePeerIdParsesAsDecimalSteamId64()
        {
            var peerId = new P2pServicePeerId("76561198000000000");

            var steamId64 = ulong.Parse(peerId.Value, CultureInfo.InvariantCulture);

            Assert.AreEqual(76561198000000000UL, steamId64);
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
