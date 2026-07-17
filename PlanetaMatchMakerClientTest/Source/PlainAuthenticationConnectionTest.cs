using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Threading.Tasks;
using CdecPGL.MinimalSerializer;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace PlanetaGameLabo.MatchMaker.Test
{
    [TestClass]
    [TestCategory("Authentication")]
    public class PlainAuthenticationConnectionTest
    {
        [TestMethod]
        public async Task ConnectAsyncSendsAuthenticationRequestAndCredentialChunks()
        {
            var token = new string('x', 300);
            using (var server = new PlainAuthenticationServer("plain-game", "1.0.0", 11))
            using (var client = new MatchMakerClient(
                       new GameId("plain-game"),
                       new GameVersion("1.0.0"),
                       3000,
                       30,
                       StreamLogger.CreateNullLogger()))
            {
                var fullName = await client.ConnectAsync(
                    new Host("127.0.0.1"),
                    new ServerPort((ushort)server.Port),
                    new PlayerName("plain-player"),
                    AuthenticationOptions.Oidc(token),
                    new ConnectionOptions(ConnectionMode.Plain));

                Assert.AreEqual("plain-player", fullName.Name);
                Assert.AreEqual((ushort)11, fullName.Tag);

                var received = await WithTimeout(server.AuthenticationRequest);
                Assert.AreEqual(ClientConstants.ApiVersion, received.Request.ApiVersion);
                Assert.AreEqual(AuthenticationMethod.Oidc, received.Request.AuthenticationMethod);
                Assert.AreEqual("plain-game", received.Request.GameId);
                Assert.AreEqual("1.0.0", received.Request.GameVersion);
                Assert.AreEqual("plain-player", received.Request.PlayerName);
                CollectionAssert.AreEqual(Encoding.UTF8.GetBytes(token), received.Credential);
                CollectionAssert.AreEqual(new ushort[] { 0, 1 }, received.Sequences.ToArray());

                client.Close();
                await WithTimeout(server.Completion);
            }
        }

        [TestMethod]
        public async Task ConnectAsyncWithoutAuthenticationSendsNoneWithNoCredential()
        {
            using (var server = new PlainAuthenticationServer("plain-game", "1.0.0", 12))
            using (var client = new MatchMakerClient(
                       new GameId("plain-game"),
                       new GameVersion("1.0.0"),
                       3000,
                       30,
                       StreamLogger.CreateNullLogger()))
            {
                var fullName = await client.ConnectAsync(
                    new Host("127.0.0.1"),
                    new ServerPort((ushort)server.Port),
                    new PlayerName("plain-player"),
                    new ConnectionOptions(ConnectionMode.Plain));

                Assert.AreEqual("plain-player", fullName.Name);
                Assert.AreEqual((ushort)12, fullName.Tag);

                var received = await WithTimeout(server.AuthenticationRequest);
                Assert.AreEqual(AuthenticationMethod.None, received.Request.AuthenticationMethod);
                Assert.AreEqual(0, received.Credential.Length);
                Assert.AreEqual(0, received.Sequences.Count);

                client.Close();
                await WithTimeout(server.Completion);
            }
        }

        private static async Task<T> WithTimeout<T>(Task<T> task)
        {
            var completedTask = await Task.WhenAny(task, Task.Delay(5000));
            if (!ReferenceEquals(completedTask, task))
            {
                Assert.Fail("Timed out waiting for plain authentication test task.");
            }

            return await task;
        }

        private static async Task WithTimeout(Task task)
        {
            var completedTask = await Task.WhenAny(task, Task.Delay(5000));
            if (!ReferenceEquals(completedTask, task))
            {
                Assert.Fail("Timed out waiting for plain authentication test task.");
            }

            await task;
        }

        private sealed class ReceivedAuthentication
        {
            public AuthenticationRequestMessage Request { get; set; }

            public byte[] Credential { get; set; }

            public List<ushort> Sequences { get; } = new List<ushort>();
        }

        private sealed class PlainAuthenticationServer : IDisposable
        {
            private readonly string expectedGameId;
            private readonly string expectedGameVersion;
            private readonly ushort playerTag;
            private readonly TcpListener listener;
            private readonly TaskCompletionSource<ReceivedAuthentication> authenticationRequest =
                new TaskCompletionSource<ReceivedAuthentication>(TaskCreationOptions.RunContinuationsAsynchronously);
            private readonly Task completion;

            public PlainAuthenticationServer(string expectedGameId, string expectedGameVersion, ushort playerTag)
            {
                this.expectedGameId = expectedGameId;
                this.expectedGameVersion = expectedGameVersion;
                this.playerTag = playerTag;
                listener = new TcpListener(IPAddress.Loopback, 0);
                listener.Start();
                Port = ((IPEndPoint)listener.LocalEndpoint).Port;
                completion = RunAsync();
            }

            public int Port { get; }

            public Task<ReceivedAuthentication> AuthenticationRequest => authenticationRequest.Task;

            public Task Completion => completion;

            public void Dispose()
            {
                listener.Stop();
            }

            private async Task RunAsync()
            {
                try
                {
                    using (var tcpClient = await listener.AcceptTcpClientAsync().ConfigureAwait(false))
                    using (var stream = tcpClient.GetStream())
                    {
                        var requestHeader = await ReadStructAsync<RequestMessageHeader>(stream).ConfigureAwait(false);
                        Assert.AreEqual(MessageType.Authentication, requestHeader.MessageType);

                        var request = await ReadStructAsync<AuthenticationRequestMessage>(stream).ConfigureAwait(false);
                        Assert.AreEqual(expectedGameId, request.GameId);
                        Assert.AreEqual(expectedGameVersion, request.GameVersion);

                        var received = new ReceivedAuthentication { Request = request };
                        var credential = new List<byte>();
                        while (credential.Count < requestHeader.AttachmentSize)
                        {
                            var chunk = await ReadStructAsync<MessageAttachmentChunk>(stream)
                                .ConfigureAwait(false);
                            var expectedDataSize = Math.Min(240, (int)requestHeader.AttachmentSize - credential.Count);
                            Assert.AreEqual((ushort)received.Sequences.Count, chunk.Sequence);
                            Assert.AreEqual((byte)expectedDataSize, chunk.DataSize);
                            received.Sequences.Add(chunk.Sequence);
                            credential.AddRange(chunk.Data.Take(chunk.DataSize));
                        }

                        received.Credential = credential.ToArray();
                        authenticationRequest.SetResult(received);

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

            private static async Task<T> ReadStructAsync<T>(Stream stream)
            {
                var buffer = new byte[Serializer.GetSerializedSize<T>()];
                var offset = 0;
                while (offset < buffer.Length)
                {
                    var readSize = await stream.ReadAsync(buffer, offset, buffer.Length - offset).ConfigureAwait(false);
                    if (readSize == 0)
                    {
                        throw new IOException("Connection closed while reading plain authentication test data.");
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
