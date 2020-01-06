using System;
using System.Collections.Generic;
using System.Net.Sockets;
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
        /// <param name="client"></param>
        /// <param name="messageBody"></param>
        /// <param name="sessionKey"></param>
        /// <exception cref="MessageInternalErrorException">Failed to receive a message.</exception>
        /// <exception cref="ObjectDisposedException">The Socket has been closed.</exception>
        /// <exception cref="SocketException">Is is possible to reach timeout</exception>
        /// <returns></returns>
        internal static async Task SendRequestMessage<T>(this TcpClient client, T messageBody, uint sessionKey = 0)
        {
            var messageAttribute = messageBody.GetType().GetCustomAttribute<MessageAttribute>();
            if (messageAttribute == null)
            {
                throw new MessageInternalErrorException(
                    "The message class is invalid because it doesn't have MessageAttribute.");
            }

            try
            {
                var header = new RequestMessageHeader
                {
                    MessageType = messageAttribute.MessageType, SessionKey = sessionKey
                };
                var requestHeaderData = new ArraySegment<byte>(Serializer.Serialize(header));
                var requestBodyData = new ArraySegment<byte>(Serializer.Serialize(messageBody));
                var data = new List<ArraySegment<byte>> {requestHeaderData, requestBodyData};
                await client.Client.SendAsync(data, SocketFlags.None).ConfigureAwait(false);
            }
            catch (InvalidSerializationException e)
            {
                throw new MessageInternalErrorException("Failed to serialize a message: " + e.Message);
            }
        }

        /// <summary>
        /// Receive a reply message from the server.
        /// </summary>
        /// <typeparam name="T">A type of message</typeparam>
        /// <param name="client"></param>
        /// <exception cref="MessageInternalErrorException">Failed to receive a message.</exception>
        /// <exception cref="ObjectDisposedException">The Socket has been closed.</exception>
        /// <exception cref="SocketException">Is is possible to reach timeout</exception>
        /// <returns></returns>
        internal static async Task<(MessageErrorCode, T)> ReceiveReplyMessage<T>(this TcpClient client)
        {
            var messageAttribute = typeof(T).GetCustomAttribute<MessageAttribute>();
            if (messageAttribute == null)
            {
                throw new MessageInternalErrorException(
                    "The message class is invalid because it doesn't have MessageAttribute.");
            }

            try
            {
                var buffer =
                    new ArraySegment<byte>(new byte[Serializer.GetSerializedSize<ReplyMessageHeader>()]);
                await client.Client.ReceiveAsync(buffer, SocketFlags.None).ConfigureAwait(false);
                var header = Serializer.Deserialize<ReplyMessageHeader>(buffer.Array);

                if (header.MessageType != messageAttribute.MessageType)
                {
                    throw new MessageInternalErrorException(
                        $"The type of received message is invalid. (expected: {messageAttribute.MessageType}, actual: {header.MessageType})");
                }

                // Receive body data even if reply code is not OK to prevent remaining body data in receive buffer.
                buffer = new ArraySegment<byte>(new byte[Serializer.GetSerializedSize<T>()]);
                await client.Client.ReceiveAsync(buffer, SocketFlags.None).ConfigureAwait(false);
                if (header.ErrorCode != MessageErrorCode.Ok)
                {
                    return (header.ErrorCode, default);
                }

                var body = Serializer.Deserialize<T>(buffer.Array);
                return (MessageErrorCode.Ok, body);
            }
            catch (InvalidSerializationException e)
            {
                throw new MessageInternalErrorException("Failed to deserialize a message: " + e.Message);
            }
        }
    }
}
#pragma warning restore CA1303