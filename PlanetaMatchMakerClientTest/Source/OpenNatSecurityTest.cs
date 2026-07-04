using System;
using System.Collections.Generic;
using System.IO;
using System.Net;
using System.Reflection;
using System.Text;
using System.Xml;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Open.Nat;

namespace PlanetaMatchMakerClientTest
{
    [TestClass]
    public class OpenNatSecurityTest
    {
        [TestMethod]
        public void GetXmlDocumentRejectsDtd()
        {
            const string xml = "<!DOCTYPE root [<!ENTITY xxe SYSTEM \"file:///etc/passwd\">]><root>&xxe;</root>";

            Assert.ThrowsException<XmlException>(() => StreamExtensions.GetXmlDocument(xml));
        }

        [TestMethod]
        public void GetXmlDocumentRejectsOversizedXml()
        {
            var xml = "<root>" + new string('a', StreamExtensions.MaxXmlResponseCharacters + 1) + "</root>";

            Assert.ThrowsException<XmlException>(() => StreamExtensions.GetXmlDocument(xml));
        }

        [TestMethod]
        public void ReadXmlResponseBodyRejectsOversizedXmlWhileReading()
        {
            var response = new StringWebResponse(
                new string('a', StreamExtensions.MaxXmlResponseCharacters + 1),
                -1);

            Assert.ThrowsException<InvalidDataException>(() => response.ReadXmlResponseBody());
        }

        [TestMethod]
        public void ReadXmlResponseBodyAllowsExactlyMaxXml()
        {
            var response = new StringWebResponse(
                new string('a', StreamExtensions.MaxXmlResponseCharacters),
                -1);

            Assert.AreEqual(StreamExtensions.MaxXmlResponseCharacters, response.ReadXmlResponseBody().Length);
        }

        [TestMethod]
        public void AnalyseReceivedResponseIgnoresLocationFromDifferentHost()
        {
            var searcher = new UpnpSearcher(new EmptyIpAddressesProvider());
            var response = Encoding.UTF8.GetBytes(
                "HTTP/1.1 200 OK\r\n" +
                "ST: urn:schemas-upnp-org:service:WANIPConnection:1\r\n" +
                "LOCATION: http://127.0.0.1:12345/root.xml\r\n\r\n");

            var device = searcher.AnalyseReceivedResponse(
                IPAddress.Parse("192.168.1.10"),
                response,
                new IPEndPoint(IPAddress.Parse("192.168.1.1"), 1900));

            Assert.IsNull(device);
        }

        [TestMethod]
        public void LocationValidationAcceptsIpv4MappedResponseAddress()
        {
            var locationUri = GetValidatedLocationUri(
                "http://192.168.1.1/root.xml",
                IPAddress.Parse("::ffff:192.168.1.1"));

            Assert.AreEqual(new Uri("http://192.168.1.1/root.xml"), locationUri);
        }

        [TestMethod]
        public void LocationValidationAcceptsScopedIpv6ResponseAddress()
        {
            var locationUri = GetValidatedLocationUri(
                "http://[fe80::1]/root.xml",
                IPAddress.Parse("fe80::1%12"));

            Assert.AreEqual(new Uri("http://[fe80::1]/root.xml"), locationUri);
        }

        [TestMethod]
        public void LocationValidationRejectsUserInfo()
        {
            var locationUri = GetValidatedLocationUri(
                "http://192.168.1.1@10.0.0.1/root.xml",
                IPAddress.Parse("10.0.0.1"));

            Assert.IsNull(locationUri);
        }

        [TestMethod]
        public void LocationValidationRejectsNonCanonicalIpv4Hosts()
        {
            var locations = new[]
            {
                ("http://010.000.000.001/root.xml", IPAddress.Parse("8.0.0.1")),
                ("http://0x0a000001/root.xml", IPAddress.Parse("10.0.0.1"))
            };

            foreach (var location in locations)
            {
                Assert.IsNull(GetValidatedLocationUri(location.Item1, location.Item2));
            }
        }

        [TestMethod]
        public void LocationValidationRejectsIpv4MappedIpv6Host()
        {
            var locationUri = GetValidatedLocationUri(
                "http://[::ffff:192.168.1.1]/root.xml",
                IPAddress.Parse("192.168.1.1"));

            Assert.IsNull(locationUri);
        }

        [TestMethod]
        public void UpnpServiceTypeRejectsPartialUrnMatches()
        {
            var method = typeof(UpnpSearcher).GetMethod(
                "TryGetValidControllerService",
                BindingFlags.Static | BindingFlags.NonPublic);
            var args = new object[]
            {
                "urn:schemas-upnp-org:service:WANIPConnection:1\" injected=\"true",
                null
            };

            var isValid = (bool)method.Invoke(null, args);

            Assert.IsFalse(isValid);
            Assert.IsNull(args[1]);
        }

        [TestMethod]
        public void UpnpServiceTypeNormalizesValidUrn()
        {
            var method = typeof(UpnpSearcher).GetMethod(
                "TryGetValidControllerService",
                BindingFlags.Static | BindingFlags.NonPublic);
            var args = new object[]
            {
                " URN:SCHEMAS-UPNP-ORG:SERVICE:WANIPCONNECTION:1 ",
                null
            };

            var isValid = (bool)method.Invoke(null, args);

            Assert.IsTrue(isValid);
            Assert.AreEqual("urn:schemas-upnp-org:service:WANIPConnection:1", args[1]);
        }

        [TestMethod]
        public void UpnpNatDeviceInfoRejectsNetworkPathControlUrl()
        {
            Assert.ThrowsException<ArgumentException>(() => new UpnpNatDeviceInfo(
                IPAddress.Parse("192.168.1.10"),
                new Uri("http://192.168.1.1:12345/root.xml"),
                "//127.0.0.1/control",
                "urn:schemas-upnp-org:service:WANIPConnection:1"));
        }

        [TestMethod]
        public void UpnpNatDeviceInfoKeepsAbsoluteControlUrlOnDiscoveredHost()
        {
            var deviceInfo = new UpnpNatDeviceInfo(
                IPAddress.Parse("192.168.1.10"),
                new Uri("http://192.168.1.1:12345/root.xml"),
                "http://127.0.0.1/ignored-host-control?x=1",
                "urn:schemas-upnp-org:service:WANIPConnection:1");

            Assert.AreEqual(new Uri("http://192.168.1.1:12345/ignored-host-control?x=1"), deviceInfo.ServiceControlUri);
        }

        [TestMethod]
        public void BuildMessageBodyEscapesXmlValues()
        {
            var mapping = new Mapping(Protocol.Tcp, 1234, 5678, "a&b<c>\"'");
            var requestMessage = new CreatePortMappingRequestMessage(mapping);
            var soapClient = new SoapClient(
                new Uri("http://192.168.1.1:12345/control"),
                "urn:schemas-upnp-org:service:WANIPConnection:1");

            var method = typeof(SoapClient).GetMethod("BuildMessageBody", BindingFlags.Instance | BindingFlags.NonPublic);
            var body = Encoding.UTF8.GetString((byte[])method.Invoke(
                soapClient,
                new object[] { "AddPortMapping", requestMessage.ToXml() }));

            StringAssert.Contains(
                body,
                "<NewPortMappingDescription>a&amp;b&lt;c&gt;&quot;&apos;</NewPortMappingDescription>");
        }

        [TestMethod]
        public void SoapRequestDisablesRedirects()
        {
            var soapClient = new SoapClient(
                new Uri("http://192.168.1.1:12345/control"),
                "urn:schemas-upnp-org:service:WANIPConnection:1");
            var method = typeof(SoapClient).GetMethod(
                "BuildHttpWebRequest",
                BindingFlags.Instance | BindingFlags.NonPublic);

            var request = (HttpWebRequest)method.Invoke(soapClient, new object[] { "AddPortMapping", Array.Empty<byte>() });

            Assert.IsFalse(request.AllowAutoRedirect);
        }

        private static Uri GetValidatedLocationUri(string location, IPAddress responseAddress)
        {
            var method = typeof(UpnpSearcher).GetMethod(
                "GetValidatedLocationUri",
                BindingFlags.Static | BindingFlags.NonPublic);
            var message = new DiscoveryResponseMessage(
                "HTTP/1.1 200 OK\r\n" +
                "LOCATION: " + location + "\r\n\r\n");

            return (Uri)method.Invoke(null, new object[] { message, new IPEndPoint(responseAddress, 1900) });
        }

        private sealed class EmptyIpAddressesProvider : IIPAddressesProvider
        {
            public IEnumerable<IPAddress> UnicastAddresses()
            {
                return Array.Empty<IPAddress>();
            }
        }

        private sealed class StringWebResponse : WebResponse
        {
            private readonly byte[] responseBody;
            private readonly long contentLength;

            public StringWebResponse(string responseBody, long contentLength)
            {
                this.responseBody = Encoding.UTF8.GetBytes(responseBody);
                this.contentLength = contentLength;
            }

            public override long ContentLength
            {
                get { return contentLength; }
                set { throw new NotSupportedException(); }
            }

            public override Stream GetResponseStream()
            {
                return new MemoryStream(responseBody);
            }
        }
    }
}
