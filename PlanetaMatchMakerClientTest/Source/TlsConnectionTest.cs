using System;
using System.IO;
using System.Net;
using System.Net.Security;
using System.Net.Sockets;
using System.Security.Authentication;
using System.Security.Cryptography;
using System.Security.Cryptography.X509Certificates;
using System.Threading.Tasks;
using CdecPGL.MinimalSerializer;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace PlanetaGameLabo.MatchMaker.Test
{
    [TestClass]
    [TestCategory("TLS")]
    public class TlsConnectionTest
    {
        [TestMethod]
        public async Task ConnectAsyncWithTlsAuthenticatesWithSelfSignedCertificate()
        {
            using (var server = new TlsAuthenticationServer("tls-game", "1.0.0", 7))
            using (var client = new MatchMakerClient(
                       new GameId("tls-game"),
                       new GameVersion("1.0.0"),
                       3000,
                       30,
                       StreamLogger.CreateNullLogger()))
            {
                PlayerFullName fullName;
                try
                {
                    fullName = await client.ConnectAsync(
                        new Host("127.0.0.1"),
                        new ServerPort((ushort)server.Port),
                        new PlayerName("tls-player"),
                        AuthenticationOptions.Oidc("tls-test-token"),
                        new ConnectionOptions(
                            ConnectionMode.Tls,
                            new Host("localhost"),
                            (sender, certificate, chain, sslPolicyErrors) => true));
                }
                catch (Exception clientException)
                {
                    try
                    {
                        await WithTimeout(server.Completion);
                    }
                    catch (Exception serverException)
                    {
                        throw new AssertFailedException(
                            "TLS client connection failed: " + clientException +
                            Environment.NewLine + "Server task also failed or timed out: " + serverException,
                            clientException);
                    }

                    throw;
                }

                Assert.AreEqual("tls-player", fullName.Name);
                Assert.AreEqual((ushort)7, fullName.Tag);

                var request = await WithTimeout(server.AuthenticationRequest);
                Assert.AreEqual(ClientConstants.ApiVersion, request.ApiVersion);
                Assert.AreEqual(AuthenticationMethod.Oidc, request.AuthenticationMethod);
                Assert.AreEqual("tls-game", request.GameId);
                Assert.AreEqual("1.0.0", request.GameVersion);
                Assert.AreEqual("tls-player", request.PlayerName);

                client.Close();
                await WithTimeout(server.Completion);
            }
        }

        private static async Task<T> WithTimeout<T>(Task<T> task)
        {
            var completedTask = await Task.WhenAny(task, Task.Delay(5000));
            if (!ReferenceEquals(completedTask, task))
            {
                Assert.Fail("Timed out waiting for TLS authentication test task.");
            }

            return await task;
        }

        private static async Task WithTimeout(Task task)
        {
            var completedTask = await Task.WhenAny(task, Task.Delay(5000));
            if (!ReferenceEquals(completedTask, task))
            {
                Assert.Fail("Timed out waiting for TLS authentication test task.");
            }

            await task;
        }

        private sealed class TlsAuthenticationServer : IDisposable
        {
            private readonly string expectedGameId;
            private readonly string expectedGameVersion;
            private readonly ushort playerTag;
            private readonly TcpListener listener;
            private readonly X509Certificate2 certificate;
            private readonly TaskCompletionSource<AuthenticationRequestMessage> authenticationRequest =
                new TaskCompletionSource<AuthenticationRequestMessage>(TaskCreationOptions.RunContinuationsAsynchronously);
            private readonly Task completion;

            public TlsAuthenticationServer(string expectedGameId, string expectedGameVersion, ushort playerTag)
            {
                this.expectedGameId = expectedGameId;
                this.expectedGameVersion = expectedGameVersion;
                this.playerTag = playerTag;
                certificate = CreateSelfSignedCertificate();
                listener = new TcpListener(IPAddress.Loopback, 0);
                listener.Start();
                Port = ((IPEndPoint)listener.LocalEndpoint).Port;
                completion = RunAsync();
            }

            public int Port { get; }

            public Task<AuthenticationRequestMessage> AuthenticationRequest => authenticationRequest.Task;

            public Task Completion => completion;

            public void Dispose()
            {
                listener.Stop();
                certificate.Dispose();
            }

            private async Task RunAsync()
            {
                try
                {
                    using (var tcpClient = await listener.AcceptTcpClientAsync().ConfigureAwait(false))
                    using (var stream = new SslStream(tcpClient.GetStream(), false))
                    {
                        await stream.AuthenticateAsServerAsync(
                            certificate,
                            false,
                            SslProtocols.None,
                            false).ConfigureAwait(false);

                        var requestHeader = await ReadStructAsync<RequestMessageHeader>(stream).ConfigureAwait(false);
                        Assert.AreEqual(MessageType.Authentication, requestHeader.MessageType);
                        Assert.AreEqual((uint)14, requestHeader.AttachmentSize);

                        var request = await ReadStructAsync<AuthenticationRequestMessage>(stream).ConfigureAwait(false);
                        Assert.AreEqual(expectedGameId, request.GameId);
                        Assert.AreEqual(expectedGameVersion, request.GameVersion);
                        var chunk = await ReadStructAsync<MessageAttachmentChunk>(stream)
                            .ConfigureAwait(false);
                        Assert.AreEqual((ushort)0, chunk.Sequence);
                        Assert.AreEqual((byte)requestHeader.AttachmentSize, chunk.DataSize);
                        authenticationRequest.SetResult(request);

                        await WriteStructAsync(
                            stream,
                            new ReplyMessageHeader
                            {
                                MessageType = MessageType.Authentication,
                                ErrorCode = MessageErrorCode.Ok
                            }).ConfigureAwait(false);
                        await WriteStructAsync(
                            stream,
                            new AuthenticationReplyMessage
                            {
                                Result = AuthenticationResult.Success,
                                ApiVersion = ClientConstants.ApiVersion,
                                GameVersion = expectedGameVersion,
                                PlayerTag = playerTag
                            }).ConfigureAwait(false);
                        await stream.FlushAsync().ConfigureAwait(false);
                    }
                }
                catch (Exception e)
                {
                    authenticationRequest.TrySetException(e);
                    throw;
                }
            }

            private static X509Certificate2 CreateSelfSignedCertificate()
            {
                var rsa = RSA.Create(2048);
                var request = new CertificateRequest(
                    "CN=localhost",
                    rsa,
                    HashAlgorithmName.SHA256,
                    RSASignaturePadding.Pkcs1);
                request.CertificateExtensions.Add(new X509BasicConstraintsExtension(false, false, 0, false));
                request.CertificateExtensions.Add(new X509KeyUsageExtension(
                    X509KeyUsageFlags.DigitalSignature | X509KeyUsageFlags.KeyEncipherment,
                    false));
                var subjectAlternativeName = new SubjectAlternativeNameBuilder();
                subjectAlternativeName.AddDnsName("localhost");
                subjectAlternativeName.AddIpAddress(IPAddress.Loopback);
                request.CertificateExtensions.Add(subjectAlternativeName.Build());

                using (var certificate = request.CreateSelfSigned(
                    DateTimeOffset.UtcNow.AddDays(-1),
                    DateTimeOffset.UtcNow.AddDays(1)))
                {
                    return X509CertificateLoader.LoadPkcs12(
                        certificate.Export(X509ContentType.Pfx),
                        (string)null,
                        X509KeyStorageFlags.MachineKeySet | X509KeyStorageFlags.Exportable);
                }
            }

            private static async Task<T> ReadStructAsync<T>(Stream stream)
            {
                var buffer = new byte[Serializer.GetSerializedSize<T>()];
                var offset = 0;
                while (offset < buffer.Length)
                {
                    var readSize = await stream.ReadAsync(buffer, offset, buffer.Length - offset).ConfigureAwait(false);
                    if (readSize == 0)
                    {
                        throw new IOException("Connection closed while reading TLS authentication test data.");
                    }

                    offset += readSize;
                }

                return Serializer.Deserialize<T>(buffer);
            }

            private static async Task WriteStructAsync<T>(Stream stream, T value)
            {
                var buffer = Serializer.Serialize(value);
                await stream.WriteAsync(buffer, 0, buffer.Length).ConfigureAwait(false);
            }
        }
    }
}
