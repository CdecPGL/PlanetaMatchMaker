using System;
using System.IO;
using System.Linq;
using System.Threading;
using System.Threading.Tasks;
using CdecPGL.MinimalSerializer;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace PlanetaGameLabo.MatchMaker.Test
{
    [TestClass]
    public class MessageAttachmentTimeoutTest
    {
        [TestMethod]
        public async Task SendingAttachmentUsesOneAbsoluteDeadline()
        {
            var request = new AuthenticationRequestMessage
            {
                GameId = string.Empty,
                GameVersion = string.Empty,
                PlayerName = string.Empty
            };
            var messageSize = Serializer.GetSerializedSize<RequestMessageHeader>() +
                              Serializer.GetSerializedSize<AuthenticationRequestMessage>();
            using (var stream = new WritePrefixThenBlockStream(messageSize))
            {
                await Assert.ThrowsExceptionAsync<TimeoutException>(() =>
                    stream.SendRequestMessage(request, new byte[] { 1 }, 50));
                Assert.AreEqual(messageSize, stream.WrittenByteCount);
            }
        }

        [TestMethod]
        public async Task ReceivingAttachmentUsesOneAbsoluteDeadline()
        {
            var header = Serializer.Serialize(new ReplyMessageHeader
            {
                MessageType = MessageType.Authentication,
                ErrorCode = MessageErrorCode.Ok,
                AttachmentSize = 1
            });
            var body = Serializer.Serialize(new AuthenticationReplyMessage
            {
                Result = AuthenticationResult.Success,
                GameVersion = string.Empty
            });
            using (var stream = new ReadPrefixThenBlockStream(header.Concat(body).ToArray()))
            {
                await Assert.ThrowsExceptionAsync<TimeoutException>(() =>
                    stream.ReceiveReplyMessage<AuthenticationReplyMessage>(50));
            }
        }

        private sealed class WritePrefixThenBlockStream : Stream
        {
            internal WritePrefixThenBlockStream(int writableByteCount)
            {
                this.writableByteCount = writableByteCount;
            }

            internal int WrittenByteCount { get; private set; }

            private readonly int writableByteCount;

            public override bool CanRead => false;
            public override bool CanSeek => false;
            public override bool CanWrite => true;
            public override long Length => throw new NotSupportedException();

            public override long Position
            {
                get => throw new NotSupportedException();
                set => throw new NotSupportedException();
            }

            public override void Flush()
            {
            }

            public override Task FlushAsync(CancellationToken cancellationToken)
            {
                return Task.CompletedTask;
            }

            public override int Read(byte[] buffer, int offset, int count)
            {
                throw new NotSupportedException();
            }

            public override long Seek(long offset, SeekOrigin origin)
            {
                throw new NotSupportedException();
            }

            public override void SetLength(long value)
            {
                throw new NotSupportedException();
            }

            public override void Write(byte[] buffer, int offset, int count)
            {
                throw new NotSupportedException();
            }

            public override Task WriteAsync(byte[] buffer, int offset, int count,
                CancellationToken cancellationToken)
            {
                if (WrittenByteCount + count <= writableByteCount)
                {
                    WrittenByteCount += count;
                    return Task.CompletedTask;
                }

                return Task.Delay(Timeout.Infinite, cancellationToken);
            }
        }

        private sealed class ReadPrefixThenBlockStream : Stream
        {
            internal ReadPrefixThenBlockStream(byte[] prefix)
            {
                this.prefix = prefix;
            }

            private readonly byte[] prefix;
            private int position;

            public override bool CanRead => true;
            public override bool CanSeek => false;
            public override bool CanWrite => false;
            public override long Length => throw new NotSupportedException();

            public override long Position
            {
                get => throw new NotSupportedException();
                set => throw new NotSupportedException();
            }

            public override void Flush()
            {
            }

            public override int Read(byte[] buffer, int offset, int count)
            {
                throw new NotSupportedException();
            }

            public override async Task<int> ReadAsync(byte[] buffer, int offset, int count,
                CancellationToken cancellationToken)
            {
                if (position < prefix.Length)
                {
                    var copySize = Math.Min(count, prefix.Length - position);
                    Array.Copy(prefix, position, buffer, offset, copySize);
                    position += copySize;
                    return copySize;
                }

                await Task.Delay(Timeout.Infinite, cancellationToken).ConfigureAwait(false);
                return 0;
            }

            public override long Seek(long offset, SeekOrigin origin)
            {
                throw new NotSupportedException();
            }

            public override void SetLength(long value)
            {
                throw new NotSupportedException();
            }

            public override void Write(byte[] buffer, int offset, int count)
            {
                throw new NotSupportedException();
            }
        }
    }
}
