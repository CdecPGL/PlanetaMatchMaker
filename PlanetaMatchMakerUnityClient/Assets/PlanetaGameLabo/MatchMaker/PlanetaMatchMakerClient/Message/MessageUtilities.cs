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
            var messageAttribute = messageBody.GetType().GetCustomAttribute<MessageAttribute>() ??
                                   throw new MessageErrorException(
                                       "The message class is invalid because it doesn't have MessageAttribute.");

            try
            {
                var header = new RequestMessageHeader
                {
                    MessageType = messageAttribute.MessageType
                };
                var requestHeaderData = Serializer.Serialize(header);
                var requestBodyData = Serializer.Serialize(messageBody);
                await stream.WriteAsync(requestHeaderData, 0, requestHeaderData.Length).ConfigureAwait(false);
                await stream.WriteAsync(requestBodyData, 0, requestBodyData.Length).ConfigureAwait(false);
                await stream.FlushAsync().ConfigureAwait(false);
            }
            catch (InvalidSerializationException e)
            {
                throw new MessageErrorException("Failed to serialize a message: " + e.Message);
            }
        }

        internal static async Task SendAuthenticationRequestMessage(
            this Stream stream,
            AuthenticationRequestMessage messageBody,
            byte[] credential)
        {
            if (credential == null)
            {
                throw new ArgumentNullException(nameof(credential));
            }

            try
            {
                var header = new RequestMessageHeader
                {
                    MessageType = MessageType.Authentication
                };
                var requestHeaderData = Serializer.Serialize(header);
                var requestBodyData = Serializer.Serialize(messageBody);
                await stream.WriteAsync(requestHeaderData, 0, requestHeaderData.Length).ConfigureAwait(false);
                await stream.WriteAsync(requestBodyData, 0, requestBodyData.Length).ConfigureAwait(false);

                var sequence = 0;
                for (var offset = 0; offset < credential.Length; offset += AuthenticationCredentialChunkDataSize)
                {
                    var dataSize = Math.Min(AuthenticationCredentialChunkDataSize, credential.Length - offset);
                    var data = new byte[AuthenticationCredentialChunkDataSize];
                    Array.Copy(credential, offset, data, 0, dataSize);
                    var chunk = new AuthenticationCredentialChunkMessage
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
        internal static async Task<(MessageErrorCode, T?)> ReceiveReplyMessage<T>(this Stream stream)
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

                // Bodies are not sent if error
                if (header.ErrorCode != MessageErrorCode.Ok)
                {
                    return (header.ErrorCode, null);
                }

                // Receive body data even if reply code is not OK to prevent remaining body data in receive buffer.
                buffer = new byte[Serializer.GetSerializedSize<T>()];
                await ReadExactlyAsync(stream, buffer).ConfigureAwait(false);

                var body = Serializer.Deserialize<T>(buffer);
                return (MessageErrorCode.Ok, body);
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

        private const int AuthenticationCredentialChunkDataSize = 240;
    }
}
#pragma warning restore CA1303
