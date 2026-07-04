using System;
using System.Collections.Generic;
using System.IO;
using System.Net;
using System.Net.Sockets;
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

        [TestMethod]
        public void PmpDiscoveryRejectsResponseFromWrongPort()
        {
            var localAddress = IPAddress.Parse("192.168.1.20");
            var gatewayAddress = IPAddress.Parse("192.168.1.1");

            var device = AnalysePmpExternalAddressResponse(
                localAddress,
                gatewayAddress,
                CreatePmpExternalAddressResponse(PmpConstants.ResultCodeSuccess),
                new IPEndPoint(gatewayAddress, PmpConstants.ClientPort));

            Assert.IsNull(device);
        }

        [TestMethod]
        public void PmpDiscoveryRejectsResponseFromWrongGateway()
        {
            var localAddress = IPAddress.Parse("192.168.1.20");
            var gatewayAddress = IPAddress.Parse("192.168.1.1");

            var device = AnalysePmpExternalAddressResponse(
                localAddress,
                gatewayAddress,
                CreatePmpExternalAddressResponse(PmpConstants.ResultCodeSuccess),
                new IPEndPoint(IPAddress.Parse("192.168.1.2"), PmpConstants.ServerPort));

            Assert.IsNull(device);
        }

        [TestMethod]
        public void PmpDiscoveryRejectsUnexpectedLength()
        {
            var localAddress = IPAddress.Parse("192.168.1.20");
            var gatewayAddress = IPAddress.Parse("192.168.1.1");
            var response = new byte[13];
            Array.Copy(CreatePmpExternalAddressResponse(PmpConstants.ResultCodeSuccess), response, 12);

            var device = AnalysePmpExternalAddressResponse(
                localAddress,
                gatewayAddress,
                response,
                new IPEndPoint(gatewayAddress, PmpConstants.ServerPort));

            Assert.IsNull(device);
        }

        [TestMethod]
        public void PmpDiscoveryRejectsUnexpectedVersion()
        {
            var localAddress = IPAddress.Parse("192.168.1.20");
            var gatewayAddress = IPAddress.Parse("192.168.1.1");
            var response = CreatePmpExternalAddressResponse(PmpConstants.ResultCodeSuccess);
            response[0] = 1;

            var device = AnalysePmpExternalAddressResponse(
                localAddress,
                gatewayAddress,
                response,
                new IPEndPoint(gatewayAddress, PmpConstants.ServerPort));

            Assert.IsNull(device);
        }

        [TestMethod]
        public void PmpDiscoveryRejectsUnexpectedOpcode()
        {
            var localAddress = IPAddress.Parse("192.168.1.20");
            var gatewayAddress = IPAddress.Parse("192.168.1.1");
            var response = CreatePmpExternalAddressResponse(PmpConstants.ResultCodeSuccess);
            response[1] = PmpConstants.OperationExternalAddressRequest;

            var device = AnalysePmpExternalAddressResponse(
                localAddress,
                gatewayAddress,
                response,
                new IPEndPoint(gatewayAddress, PmpConstants.ServerPort));

            Assert.IsNull(device);
        }

        [TestMethod]
        public void PmpDiscoveryRejectsErrorResponse()
        {
            var localAddress = IPAddress.Parse("192.168.1.20");
            var gatewayAddress = IPAddress.Parse("192.168.1.1");

            var device = AnalysePmpExternalAddressResponse(
                localAddress,
                gatewayAddress,
                CreatePmpExternalAddressResponse(PmpConstants.ResultCodeNotAuthorized),
                new IPEndPoint(gatewayAddress, PmpConstants.ServerPort));

            Assert.IsNull(device);
        }

        [TestMethod]
        public void PmpDiscoveryRejectsUnknownErrorResponse()
        {
            var localAddress = IPAddress.Parse("192.168.1.20");
            var gatewayAddress = IPAddress.Parse("192.168.1.1");

            var device = AnalysePmpExternalAddressResponse(
                localAddress,
                gatewayAddress,
                CreatePmpExternalAddressResponse(99),
                new IPEndPoint(gatewayAddress, PmpConstants.ServerPort));

            Assert.IsNull(device);
        }

        [TestMethod]
        public void PmpDiscoveryRejectsNonPrivateLocalAddress()
        {
            var localAddress = IPAddress.Parse("203.0.113.20");
            var gatewayAddress = IPAddress.Parse("192.168.1.1");

            var device = AnalysePmpExternalAddressResponse(
                localAddress,
                gatewayAddress,
                CreatePmpExternalAddressResponse(PmpConstants.ResultCodeSuccess),
                new IPEndPoint(gatewayAddress, PmpConstants.ServerPort));

            Assert.IsNull(device);
        }

        [TestMethod]
        public void PmpDiscoveryUsesGatewayAsHostEndpoint()
        {
            var localAddress = IPAddress.Parse("192.168.1.20");
            var gatewayAddress = IPAddress.Parse("192.168.1.1");

            var device = AnalysePmpExternalAddressResponse(
                localAddress,
                gatewayAddress,
                CreatePmpExternalAddressResponse(PmpConstants.ResultCodeSuccess),
                new IPEndPoint(gatewayAddress, PmpConstants.ServerPort));

            Assert.IsNotNull(device);
            Assert.AreEqual(gatewayAddress, device.HostEndPoint.Address);
            Assert.AreEqual(PmpConstants.ServerPort, device.HostEndPoint.Port);
            Assert.AreEqual(localAddress, device.LocalAddress);
        }

        [TestMethod]
        public void PmpPortMapResponseAppliesMatchingResponse()
        {
            var gatewayAddress = IPAddress.Parse("127.0.0.2");
            var device = new PmpNatDevice(IPAddress.Loopback, gatewayAddress, IPAddress.Parse("203.0.113.10"));
            var mapping = new Mapping(Protocol.Tcp, 1234, 5678, 3600, "test");

            var accepted = TryApplyPmpPortMapResponse(
                device,
                CreatePmpPortMapResponse(Protocol.Tcp, PmpConstants.ResultCodeSuccess, 1234, 6000, 600),
                new IPEndPoint(gatewayAddress, PmpConstants.ServerPort),
                mapping);

            Assert.IsTrue(accepted);
            Assert.AreEqual(6000, mapping.PublicPort);
            Assert.AreEqual(Protocol.Tcp, mapping.Protocol);
            Assert.IsTrue(mapping.Expiration > DateTime.UtcNow.AddMinutes(9));
        }

        [TestMethod]
        public void PmpPortMapResponseIgnoresWrongEndpoint()
        {
            var gatewayAddress = IPAddress.Parse("127.0.0.2");
            var device = new PmpNatDevice(IPAddress.Loopback, gatewayAddress, IPAddress.Parse("203.0.113.10"));
            var mapping = new Mapping(Protocol.Tcp, 1234, 5678, 3600, "test");

            var accepted = TryApplyPmpPortMapResponse(
                device,
                CreatePmpPortMapResponse(Protocol.Tcp, PmpConstants.ResultCodeSuccess, 1234, 6000, 600),
                new IPEndPoint(gatewayAddress, PmpConstants.ClientPort),
                mapping);

            Assert.IsFalse(accepted);
            Assert.AreEqual(5678, mapping.PublicPort);
        }

        [TestMethod]
        public void PmpPortMapResponseIgnoresWrongGateway()
        {
            var gatewayAddress = IPAddress.Parse("127.0.0.2");
            var device = new PmpNatDevice(IPAddress.Loopback, gatewayAddress, IPAddress.Parse("203.0.113.10"));
            var mapping = new Mapping(Protocol.Tcp, 1234, 5678, 3600, "test");

            var accepted = TryApplyPmpPortMapResponse(
                device,
                CreatePmpPortMapResponse(Protocol.Tcp, PmpConstants.ResultCodeSuccess, 1234, 6000, 600),
                new IPEndPoint(IPAddress.Parse("127.0.0.3"), PmpConstants.ServerPort),
                mapping);

            Assert.IsFalse(accepted);
            Assert.AreEqual(5678, mapping.PublicPort);
        }

        [TestMethod]
        public void PmpPortMapResponseIgnoresUnexpectedVersion()
        {
            var gatewayAddress = IPAddress.Parse("127.0.0.2");
            var device = new PmpNatDevice(IPAddress.Loopback, gatewayAddress, IPAddress.Parse("203.0.113.10"));
            var mapping = new Mapping(Protocol.Tcp, 1234, 5678, 3600, "test");
            var response = CreatePmpPortMapResponse(Protocol.Tcp, PmpConstants.ResultCodeSuccess, 1234, 6000, 600);
            response[0] = 1;

            var accepted = TryApplyPmpPortMapResponse(
                device,
                response,
                new IPEndPoint(gatewayAddress, PmpConstants.ServerPort),
                mapping);

            Assert.IsFalse(accepted);
            Assert.AreEqual(5678, mapping.PublicPort);
        }

        [TestMethod]
        public void PmpPortMapResponseIgnoresUnexpectedOpcode()
        {
            var gatewayAddress = IPAddress.Parse("127.0.0.2");
            var device = new PmpNatDevice(IPAddress.Loopback, gatewayAddress, IPAddress.Parse("203.0.113.10"));
            var mapping = new Mapping(Protocol.Tcp, 1234, 5678, 3600, "test");
            var response = CreatePmpPortMapResponse(Protocol.Tcp, PmpConstants.ResultCodeSuccess, 1234, 6000, 600);
            response[1] = PmpConstants.OperationCodeTcp;

            var accepted = TryApplyPmpPortMapResponse(
                device,
                response,
                new IPEndPoint(gatewayAddress, PmpConstants.ServerPort),
                mapping);

            Assert.IsFalse(accepted);
            Assert.AreEqual(5678, mapping.PublicPort);
        }

        [TestMethod]
        public void PmpPortMapResponseIgnoresWrongProtocol()
        {
            var gatewayAddress = IPAddress.Parse("127.0.0.2");
            var device = new PmpNatDevice(IPAddress.Loopback, gatewayAddress, IPAddress.Parse("203.0.113.10"));
            var mapping = new Mapping(Protocol.Tcp, 1234, 5678, 3600, "test");

            var accepted = TryApplyPmpPortMapResponse(
                device,
                CreatePmpPortMapResponse(Protocol.Udp, PmpConstants.ResultCodeSuccess, 1234, 6000, 600),
                new IPEndPoint(gatewayAddress, PmpConstants.ServerPort),
                mapping);

            Assert.IsFalse(accepted);
            Assert.AreEqual(5678, mapping.PublicPort);
        }

        [TestMethod]
        public void PmpPortMapResponseIgnoresWrongPrivatePort()
        {
            var gatewayAddress = IPAddress.Parse("127.0.0.2");
            var device = new PmpNatDevice(IPAddress.Loopback, gatewayAddress, IPAddress.Parse("203.0.113.10"));
            var mapping = new Mapping(Protocol.Tcp, 1234, 5678, 3600, "test");

            var accepted = TryApplyPmpPortMapResponse(
                device,
                CreatePmpPortMapResponse(Protocol.Tcp, PmpConstants.ResultCodeSuccess, 4321, 6000, 600),
                new IPEndPoint(gatewayAddress, PmpConstants.ServerPort),
                mapping);

            Assert.IsFalse(accepted);
            Assert.AreEqual(5678, mapping.PublicPort);
        }

        [TestMethod]
        public void PmpPortMapResponseIgnoresUnexpectedLength()
        {
            var gatewayAddress = IPAddress.Parse("127.0.0.2");
            var device = new PmpNatDevice(IPAddress.Loopback, gatewayAddress, IPAddress.Parse("203.0.113.10"));
            var mapping = new Mapping(Protocol.Tcp, 1234, 5678, 3600, "test");
            var response = new byte[17];
            Array.Copy(
                CreatePmpPortMapResponse(Protocol.Tcp, PmpConstants.ResultCodeSuccess, 1234, 6000, 600),
                response,
                16);

            var accepted = TryApplyPmpPortMapResponse(
                device,
                response,
                new IPEndPoint(gatewayAddress, PmpConstants.ServerPort),
                mapping);

            Assert.IsFalse(accepted);
            Assert.AreEqual(5678, mapping.PublicPort);
        }

        [TestMethod]
        public void PmpPortMapResponseThrowsOnErrorResult()
        {
            var gatewayAddress = IPAddress.Parse("127.0.0.2");
            var device = new PmpNatDevice(IPAddress.Loopback, gatewayAddress, IPAddress.Parse("203.0.113.10"));
            var mapping = new Mapping(Protocol.Tcp, 1234, 5678, 3600, "test");

            var exception = Assert.ThrowsException<MappingException>(() => TryApplyPmpPortMapResponse(
                device,
                CreatePmpPortMapResponse(Protocol.Tcp, PmpConstants.ResultCodeUnsupportedOperationCode, 1234, 6000, 600),
                new IPEndPoint(gatewayAddress, PmpConstants.ServerPort),
                mapping));

            Assert.AreEqual((int)PmpConstants.ResultCodeUnsupportedOperationCode, exception.ErrorCode);
        }

        [TestMethod]
        public void PmpPortMapResponseThrowsOnUnknownErrorResult()
        {
            var gatewayAddress = IPAddress.Parse("127.0.0.2");
            var device = new PmpNatDevice(IPAddress.Loopback, gatewayAddress, IPAddress.Parse("203.0.113.10"));
            var mapping = new Mapping(Protocol.Tcp, 1234, 5678, 3600, "test");

            var exception = Assert.ThrowsException<MappingException>(() => TryApplyPmpPortMapResponse(
                device,
                CreatePmpPortMapResponse(Protocol.Tcp, 99, 1234, 6000, 600),
                new IPEndPoint(gatewayAddress, PmpConstants.ServerPort),
                mapping));

            Assert.AreEqual(99, exception.ErrorCode);
            Assert.AreEqual("Unknown NAT-PMP error", exception.ErrorText);
        }

        [TestMethod]
        public void PmpDeleteResponseAcceptsZeroPublicPortAndLifetime()
        {
            var gatewayAddress = IPAddress.Parse("127.0.0.2");
            var device = new PmpNatDevice(IPAddress.Loopback, gatewayAddress, IPAddress.Parse("203.0.113.10"));
            var mapping = new Mapping(Protocol.Tcp, 1234, 5678, 3600, "test");

            var accepted = TryApplyPmpPortMapResponse(
                device,
                CreatePmpPortMapResponse(Protocol.Tcp, PmpConstants.ResultCodeSuccess, 1234, 0, 0),
                new IPEndPoint(gatewayAddress, PmpConstants.ServerPort),
                mapping,
                false);

            Assert.IsTrue(accepted);
            Assert.AreEqual(5678, mapping.PublicPort);
        }

        [TestMethod]
        public void PmpDeleteResponseRejectsNonZeroPublicPortOrLifetime()
        {
            var gatewayAddress = IPAddress.Parse("127.0.0.2");
            var device = new PmpNatDevice(IPAddress.Loopback, gatewayAddress, IPAddress.Parse("203.0.113.10"));
            var mapping = new Mapping(Protocol.Tcp, 1234, 5678, 3600, "test");

            var nonZeroPublicPortAccepted = TryApplyPmpPortMapResponse(
                device,
                CreatePmpPortMapResponse(Protocol.Tcp, PmpConstants.ResultCodeSuccess, 1234, 6000, 0),
                new IPEndPoint(gatewayAddress, PmpConstants.ServerPort),
                mapping,
                false);
            var nonZeroLifetimeAccepted = TryApplyPmpPortMapResponse(
                device,
                CreatePmpPortMapResponse(Protocol.Tcp, PmpConstants.ResultCodeSuccess, 1234, 0, 600),
                new IPEndPoint(gatewayAddress, PmpConstants.ServerPort),
                mapping,
                false);

            Assert.IsFalse(nonZeroPublicPortAccepted);
            Assert.IsFalse(nonZeroLifetimeAccepted);
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

        private static bool TryApplyPmpPortMapResponse(
            PmpNatDevice device,
            byte[] response,
            IPEndPoint endpoint,
            Mapping mapping,
            bool create = true)
        {
            var method = typeof(PmpNatDevice).GetMethod(
                "TryApplyPortMapResponse",
                BindingFlags.Instance | BindingFlags.NonPublic);

            try
            {
                return (bool)method.Invoke(device, new object[] { response, endpoint, mapping, create });
            }
            catch (TargetInvocationException e)
            {
                throw e.InnerException;
            }
        }

        private static NatDevice AnalysePmpExternalAddressResponse(
            IPAddress localAddress,
            IPAddress expectedGatewayAddress,
            byte[] response,
            IPEndPoint endpoint)
        {
            using (var udpClient = new UdpClient())
            {
                var searcher = new PmpSearcher(new EmptyIpAddressesProvider());
                var gatewayLists = new Dictionary<UdpClient, IEnumerable<IPEndPoint>>
                {
                    { udpClient, new[] { new IPEndPoint(expectedGatewayAddress, PmpConstants.ServerPort) } }
                };
                var field = typeof(PmpSearcher).GetField(
                    "_gatewayLists",
                    BindingFlags.Instance | BindingFlags.NonPublic);

                field.SetValue(searcher, gatewayLists);
                return searcher.AnalyseReceivedResponse(localAddress, response, endpoint);
            }
        }

        private static byte[] CreatePmpExternalAddressResponse(int resultCode)
        {
            var response = new byte[12];
            response[0] = PmpConstants.Version;
            response[1] = PmpConstants.ServerNoop;
            WriteUInt16(response, 2, resultCode);
            WriteUInt32(response, 4, 1);
            response[8] = 203;
            response[9] = 0;
            response[10] = 113;
            response[11] = 10;
            return response;
        }

        private static byte[] CreatePmpPortMapResponse(
            Protocol protocol,
            int resultCode,
            int privatePort,
            int publicPort,
            uint lifetime)
        {
            var response = new byte[16];
            response[0] = PmpConstants.Version;
            response[1] = (byte)(PmpConstants.ServerNoop
                + (protocol == Protocol.Tcp ? PmpConstants.OperationCodeTcp : PmpConstants.OperationCodeUdp));
            WriteUInt16(response, 2, resultCode);
            WriteUInt32(response, 4, 1);
            WriteUInt16(response, 8, privatePort);
            WriteUInt16(response, 10, publicPort);
            WriteUInt32(response, 12, lifetime);
            return response;
        }

        private static void WriteUInt16(byte[] buffer, int offset, int value)
        {
            buffer[offset] = (byte)(value >> 8);
            buffer[offset + 1] = (byte)value;
        }

        private static void WriteUInt32(byte[] buffer, int offset, uint value)
        {
            buffer[offset] = (byte)(value >> 24);
            buffer[offset + 1] = (byte)(value >> 16);
            buffer[offset + 2] = (byte)(value >> 8);
            buffer[offset + 3] = (byte)value;
        }

        private sealed class EmptyIpAddressesProvider : IIPAddressesProvider
        {
            public IEnumerable<IPAddress> GatewayAddresses()
            {
                return Array.Empty<IPAddress>();
            }

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
