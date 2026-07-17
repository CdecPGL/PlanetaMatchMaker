using System;
using System.IO;
using System.Reflection;
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
        internal static async Task SendRequestMessage<T>(this Stream stream, T messageBody)
        {
            await SendRequestMessage(stream, messageBody, Array.Empty<byte>()).ConfigureAwait(false);
        }

        internal static async Task SendRequestMessage<T>(this Stream stream, T messageBody, byte[] attachment)
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

            try
            {
                var header = new RequestMessageHeader
                {
                    MessageType = messageAttribute.MessageType,
                    AttachmentSize = checked((uint)attachment.Length)
                };
                var requestHeaderData = Serializer.Serialize(header);
                var requestBodyData = Serializer.Serialize(messageBody);
                await stream.WriteAsync(requestHeaderData, 0, requestHeaderData.Length).ConfigureAwait(false);
                await stream.WriteAsync(requestBodyData, 0, requestBodyData.Length).ConfigureAwait(false);

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
                    await stream.WriteAsync(chunkData, 0, chunkData.Length).ConfigureAwait(false);
                    ++sequence;
                }

                await stream.FlushAsync().ConfigureAwait(false);
            }
            catch (InvalidSerializationException e)
            {
                throw new MessageErrorException("Failed to serialize a message: " + e.Message);
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
        internal static async Task<(MessageErrorCode, T?, byte[])> ReceiveReplyMessage<T>(this Stream stream)
            where T : struct
        {
            var messageAttribute = typeof(T).GetCustomAttribute<MessageAttribute>() ??
                                   throw new MessageErrorException(
                                       "The message class is invalid because it doesn't have MessageAttribute.");

            try
            {
                var buffer = new byte[Serializer.GetSerializedSize<ReplyMessageHeader>()];
                await ReadExactlyAsync(stream, buffer).ConfigureAwait(false);
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

                // Receive body data even if reply code is not OK to prevent remaining body data in receive buffer.
                buffer = new byte[Serializer.GetSerializedSize<T>()];
                await ReadExactlyAsync(stream, buffer).ConfigureAwait(false);

                var body = Serializer.Deserialize<T>(buffer);
                var attachment = await ReceiveMessageAttachment(stream, header.AttachmentSize).ConfigureAwait(false);
                return (MessageErrorCode.Ok, body, attachment);
            }
            catch (InvalidSerializationException e)
            {
                throw new MessageErrorException("Failed to deserialize a message: " + e.Message);
            }
        }

        private static async Task ReadExactlyAsync(Stream stream, byte[] buffer)
        {
            var offset = 0;
            while (offset < buffer.Length)
            {
                var readSize = await stream.ReadAsync(buffer, offset, buffer.Length - offset).ConfigureAwait(false);
                if (readSize == 0)
                {
                    throw new MessageErrorException("Connection closed unexpectedly.");
                }

                offset += readSize;
            }
        }

        private static async Task<byte[]> ReceiveMessageAttachment(Stream stream, uint attachmentSize)
        {
            var attachment = new byte[checked((int)attachmentSize)];
            var offset = 0;
            var sequence = 0;
            while (offset < attachment.Length)
            {
                var buffer = new byte[Serializer.GetSerializedSize<MessageAttachmentChunk>()];
                await ReadExactlyAsync(stream, buffer).ConfigureAwait(false);
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

        private const int MessageAttachmentChunkDataSize = 240;
    }
}
#pragma warning restore CA1303
