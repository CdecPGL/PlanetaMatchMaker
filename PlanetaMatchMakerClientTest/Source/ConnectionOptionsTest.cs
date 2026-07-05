using System;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace PlanetaGameLabo.MatchMaker.Test
{
    [TestClass]
    [TestCategory("ConnectionOptions")]
    public class ConnectionOptionsTest
    {
        [TestMethod]
        public void DefaultModeIsTls()
        {
            var options = new ConnectionOptions();

            Assert.AreEqual(ConnectionMode.Tls, options.Mode);
            Assert.IsNull(options.TlsTargetHost);
            Assert.IsNull(options.RemoteCertificateValidationCallback);
        }

        [TestMethod]
        public void ModeCanBePlain()
        {
            var options = new ConnectionOptions(ConnectionMode.Plain);

            Assert.AreEqual(ConnectionMode.Plain, options.Mode);
        }

        [TestMethod]
        public void TlsTargetHostIsValidated()
        {
            var options = new ConnectionOptions(
                ConnectionMode.Tls,
                "match.example.com");

            Assert.AreEqual("match.example.com", options.TlsTargetHost);
        }

        [TestMethod]
        public void InvalidTlsTargetHostThrowsArgumentException()
        {
            Assert.ThrowsException<ArgumentException>(() =>
                new ConnectionOptions(
                    ConnectionMode.Tls,
                    "example..com"));
        }

        [TestMethod]
        public void UndefinedModeThrowsArgumentOutOfRangeException()
        {
            Assert.ThrowsException<ArgumentOutOfRangeException>(() =>
                new ConnectionOptions((ConnectionMode)255));
        }
    }
}
