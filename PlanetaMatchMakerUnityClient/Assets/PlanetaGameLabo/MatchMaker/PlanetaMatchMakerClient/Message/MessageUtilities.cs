using System;
using System.IO;
using System.Reflection;
using System.Threading;
using System.Threading.Tasks;
using CdecPGL.MinimalSerializer;

#pragma warning disable CA1303
namespace PlanetaGameLabo.MatchMaker
{
    internal static class MessageUtilities
    {
        /// <summary>
        /// Send a request or notice message to the server.
        /// </summary>
        /// <typeparam name="T">A type of message</typeparam>
        /// <param name="stream"></param>
        /// <param name="messageBody"></param>
        /// <exception cref="MessageErrorException">Failed to receive a message.</exception>
        /// <exception cref="ObjectDisposedException">The Socket has been closed.</exception>
        /// <returns></returns>
        internal static async Task SendRequestMessage<T>(this Stream stream, T messageBody,
            int timeoutMilliseconds)
        {
            await SendRequestMessage(stream, messageBody, Array.Empty<byte>(), timeoutMilliseconds)
                .ConfigureAwait(false);
        }

        internal static async Task SendRequestMessage<T>(this Stream stream, T messageBody, byte[] attachment,
            int timeoutMilliseconds)
        {
            if (attachment == null)
            {
                throw new ArgumentNullException(nameof(attachment));
            }

            if (attachment.Length > ClientConstants.MaxMessageAttachmentLength)
            {
                throw new ArgumentOutOfRangeException(nameof(attachment),
                    $"Message attachment must be at most {ClientConstants.MaxMessageAttachmentLength} bytes.");
            }

            var messageAttribute = messageBody.GetType().GetCustomAttribute<MessageAttribute>() ??
                                   throw new MessageErrorException(
                                       "The message class is invalid because it doesn't have MessageAttribute.");

            using (var deadline = CreateDeadline(timeoutMilliseconds))
            {
                try
                {
                    var header = new RequestMessageHeader
                    {
                        MessageType = messageAttribute.MessageType,
                        AttachmentSize = checked((uint)attachment.Length)
                    };
                    var requestHeaderData = Serializer.Serialize(header);
                    var requestBodyData = Serializer.Serialize(messageBody);
                    await stream.WriteAsync(requestHeaderData, 0, requestHeaderData.Length, deadline.Token)
                        .ConfigureAwait(false);
                    await stream.WriteAsync(requestBodyData, 0, requestBodyData.Length, deadline.Token)
                        .ConfigureAwait(false);

                    var sequence = 0;
                    for (var offset = 0; offset < attachment.Length; offset += MessageAttachmentChunkDataSize)
                    {
                        var dataSize = Math.Min(MessageAttachmentChunkDataSize, attachment.Length - offset);
                        var data = new byte[MessageAttachmentChunkDataSize];
                        Array.Copy(attachment, offset, data, 0, dataSize);
                        var chunk = new MessageAttachmentChunk
                        {
                            Sequence = checked((ushort)sequence),
                            DataSize = checked((byte)dataSize),
                            Data = data
                        };
                        var chunkData = Serializer.Serialize(chunk);
                        await stream.WriteAsync(chunkData, 0, chunkData.Length, deadline.Token)
                            .ConfigureAwait(false);
                        ++sequence;
                    }

                    await stream.FlushAsync(deadline.Token).ConfigureAwait(false);
                }
                catch (OperationCanceledException e) when (deadline.IsCancellationRequested)
                {
                    throw new TimeoutException("Timed out while sending a message to the server.", e);
                }
                catch (InvalidSerializationException e)
                {
                    throw new MessageErrorException("Failed to serialize a message: " + e.Message);
                }
            }
        }

        /// <summary>
        /// Receive a reply message from the server.
        /// This method won't receive body data if reply code is not OK.
        /// </summary>
        /// <typeparam name="T">A type of message</typeparam>
        /// <param name="stream"></param>
        /// <exception cref="MessageErrorException">Failed to receive a message.</exception>
        /// <exception cref="ObjectDisposedException">The Socket has been closed.</exception>
        /// <returns></returns>
        internal static async Task<(MessageErrorCode, T?, byte[])> ReceiveReplyMessage<T>(this Stream stream,
            int timeoutMilliseconds)
            where T : struct
        {
            var messageAttribute = typeof(T).GetCustomAttribute<MessageAttribute>() ??
                                   throw new MessageErrorException(
                                       "The message class is invalid because it doesn't have MessageAttribute.");

            using (var deadline = CreateDeadline(timeoutMilliseconds))
            {
                try
                {
                    var buffer = new byte[Serializer.GetSerializedSize<ReplyMessageHeader>()];
                    await ReadExactlyAsync(stream, buffer, deadline.Token).ConfigureAwait(false);
                    var header = Serializer.Deserialize<ReplyMessageHeader>(buffer);

                    if (header.MessageType != messageAttribute.MessageType)
                    {
                        throw new MessageErrorException(
                            $"The type of received message is invalid. (expected: {messageAttribute.MessageType}, actual: {header.MessageType})");
                    }

                    if (header.AttachmentSize > ClientConstants.MaxMessageAttachmentLength)
                    {
                        throw new MessageErrorException(
                            $"Reply attachment exceeds the protocol limit. (actual: {header.AttachmentSize})");
                    }

                    // Bodies are not sent if error
                    if (header.ErrorCode != MessageErrorCode.Ok)
                    {
                        if (header.AttachmentSize != 0)
                        {
                            throw new MessageErrorException("An error reply must not contain an attachment.");
                        }

                        return (header.ErrorCode, null, Array.Empty<byte>());
                    }

                    buffer = new byte[Serializer.GetSerializedSize<T>()];
                    await ReadExactlyAsync(stream, buffer, deadline.Token).ConfigureAwait(false);

                    var body = Serializer.Deserialize<T>(buffer);
                    var attachment = await ReceiveMessageAttachment(stream, header.AttachmentSize, deadline.Token)
                        .ConfigureAwait(false);
                    return (MessageErrorCode.Ok, body, attachment);
                }
                catch (OperationCanceledException e) when (deadline.IsCancellationRequested)
                {
                    throw new TimeoutException("Timed out while receiving a message from the server.", e);
                }
                catch (InvalidSerializationException e)
                {
                    throw new MessageErrorException("Failed to deserialize a message: " + e.Message);
                }
            }
        }

        private static async Task ReadExactlyAsync(Stream stream, byte[] buffer, CancellationToken cancellationToken)
        {
            var offset = 0;
            while (offset < buffer.Length)
            {
                var readSize = await stream.ReadAsync(buffer, offset, buffer.Length - offset, cancellationToken)
                    .ConfigureAwait(false);
                if (readSize == 0)
                {
                    throw new MessageErrorException("Connection closed unexpectedly.");
                }

                offset += readSize;
            }
        }

        private static async Task<byte[]> ReceiveMessageAttachment(Stream stream, uint attachmentSize,
            CancellationToken cancellationToken)
        {
            var attachment = new byte[checked((int)attachmentSize)];
            var offset = 0;
            var sequence = 0;
            while (offset < attachment.Length)
            {
                var buffer = new byte[Serializer.GetSerializedSize<MessageAttachmentChunk>()];
                await ReadExactlyAsync(stream, buffer, cancellationToken).ConfigureAwait(false);
                var chunk = Serializer.Deserialize<MessageAttachmentChunk>(buffer);
                var expectedDataSize = Math.Min(MessageAttachmentChunkDataSize, attachment.Length - offset);
                if (chunk.Sequence != sequence || chunk.DataSize != expectedDataSize)
                {
                    throw new MessageErrorException("The message attachment chunk is invalid.");
                }

                for (var i = expectedDataSize; i < chunk.Data.Length; ++i)
                {
                    if (chunk.Data[i] != 0)
                    {
                        throw new MessageErrorException("The message attachment chunk padding is invalid.");
                    }
                }

                Array.Copy(chunk.Data, 0, attachment, offset, expectedDataSize);
                offset += expectedDataSize;
                ++sequence;
            }

            return attachment;
        }

        private static CancellationTokenSource CreateDeadline(int timeoutMilliseconds)
        {
            if (timeoutMilliseconds < Timeout.Infinite)
            {
                throw new ArgumentOutOfRangeException(nameof(timeoutMilliseconds));
            }

            var deadline = new CancellationTokenSource();
            if (timeoutMilliseconds > 0)
            {
                deadline.CancelAfter(timeoutMilliseconds);
            }

            return deadline;
        }

        private const int MessageAttachmentChunkDataSize = 240;
    }
}
#pragma warning restore CA1303
