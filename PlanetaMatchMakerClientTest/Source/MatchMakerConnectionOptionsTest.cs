using System;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace PlanetaGameLabo.MatchMaker.Test
{
    [TestClass]
    [TestCategory("MatchMakerConnectionOptions")]
    public class MatchMakerConnectionOptionsTest
    {
        [TestMethod]
        public void DefaultModeIsTls()
        {
            var options = new MatchMakerConnectionOptions();

            Assert.AreEqual(MatchMakerConnectionMode.Tls, options.Mode);
            Assert.IsNull(options.TlsTargetHost);
            Assert.IsNull(options.RemoteCertificateValidationCallback);
        }

        [TestMethod]
        public void ModeCanBePlain()
        {
            var options = new MatchMakerConnectionOptions(MatchMakerConnectionMode.Plain);

            Assert.AreEqual(MatchMakerConnectionMode.Plain, options.Mode);
        }

        [TestMethod]
        public void TlsTargetHostIsValidated()
        {
            var options = new MatchMakerConnectionOptions(
                MatchMakerConnectionMode.Tls,
                "match.example.com");

            Assert.AreEqual("match.example.com", options.TlsTargetHost);
        }

        [TestMethod]
        public void InvalidTlsTargetHostThrowsArgumentException()
        {
            Assert.ThrowsException<ArgumentException>(() =>
                new MatchMakerConnectionOptions(
                    MatchMakerConnectionMode.Tls,
                    "example..com"));
        }

        [TestMethod]
        public void UndefinedModeThrowsArgumentOutOfRangeException()
        {
            Assert.ThrowsException<ArgumentOutOfRangeException>(() =>
                new MatchMakerConnectionOptions((MatchMakerConnectionMode)255));
        }
    }
}
