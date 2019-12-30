using System;

namespace PlanetaGameLabo.MatchMaker
{
    /// <summary>
    /// An exception of client due to user operation.
    /// When this exception is thrown, Connection will be continued if possible.
    /// </summary>
    public sealed class ClientErrorException : Exception
    {
        public ClientErrorCode ClientErrorCode { get; }

        public string ExtraMessage { get; }

        public ClientErrorException(ClientErrorCode errorCode) : base(errorCode.GetClientErrorMessage())
        {
            ClientErrorCode = errorCode;
        }

        public ClientErrorException(ClientErrorCode errorCode, string extraMessage) : base(
            errorCode.GetClientErrorMessage() + ": " + extraMessage)
        {
            ClientErrorCode = errorCode;
            ExtraMessage = extraMessage;
        }
    }

    /// <summary>
    /// Exception of client due to system.
    /// When this exception is thrown, the connection will be disconnected.
    /// </summary>
    public sealed class ClientInternalErrorException : Exception
    {
        public ClientInternalErrorException(string message) : base(message)
        {
        }
    }

    public enum ClientErrorCode
    {
        Ok,
        FailedToConnect,
        AlreadyConnected,
        NotConnected,
        RequestError,
        AlreadyHostingRoom,
        NotHostingRoom,
        ConnectionClosed,
        CreatingPortMappingFailed,
        NotReachable,
        UnknownError,
    };

    public static class ClientErrorCodeExtensions
    {
        public static string GetClientErrorMessage(this ClientErrorCode error_code)
        {
            switch (error_code)
            {
                case ClientErrorCode.Ok:
                    return "Ok";
                case ClientErrorCode.FailedToConnect:
                    return "Failed to connect to the server.";
                case ClientErrorCode.RequestError:
                    return "Request doesn't processed correctly on the server.";
                case ClientErrorCode.AlreadyConnected:
                    return "The client is already connected to the server.";
                case ClientErrorCode.NotConnected:
                    return "The client is not connected to the server.";
                case ClientErrorCode.AlreadyHostingRoom:
                    return "The client is already hosting a room.";
                case ClientErrorCode.NotHostingRoom:
                    return "The client is not hosting a room.";
                case ClientErrorCode.ConnectionClosed:
                    return "The connection is closed.";
                case ClientErrorCode.UnknownError:
                    return "Unexpected error.";
                case ClientErrorCode.CreatingPortMappingFailed:
                    return "Failed to create port mapping to NAT.";
                case ClientErrorCode.NotReachable:
                    return "This machine is not reachable from machines via internet.";
                default:
                    return "";
            }
        }
    }
}