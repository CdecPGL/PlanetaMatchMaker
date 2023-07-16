using System;

namespace PlanetaGameLabo.MatchMaker
{
#pragma warning disable CA1032
    /// <summary>
    /// An exception of client.
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

#pragma warning restore CA1032

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
        ConnectionEstablishModeMismatch,
        NotReachable,
        InvalidOperation,
        SystemError, // Not continuable system error
        UnknownError,
    };

    public static class ClientErrorCodeExtensions
    {
        public static string GetClientErrorMessage(this ClientErrorCode errorCode)
        {
            switch (errorCode)
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
                case ClientErrorCode.ConnectionEstablishModeMismatch:
                    return "Failed to operation because connection establish mode is different.";
                case ClientErrorCode.NotReachable:
                    return "This machine is not reachable from machines via internet.";
                case ClientErrorCode.SystemError:
                    return "System or network internal error. Not continuable.";
                case ClientErrorCode.InvalidOperation:
                    return "Requested operation is invalid.";
                default:
                    return "";
            }
        }

        public static bool IsContinuable(this ClientErrorCode errorCode)
        {
            switch (errorCode)
            {
                case ClientErrorCode.Ok:
                case ClientErrorCode.AlreadyConnected:
                case ClientErrorCode.RequestError:
                case ClientErrorCode.AlreadyHostingRoom:
                case ClientErrorCode.NotHostingRoom:
                case ClientErrorCode.ConnectionClosed:
                case ClientErrorCode.CreatingPortMappingFailed:
                case ClientErrorCode.ConnectionEstablishModeMismatch:
                case ClientErrorCode.NotReachable:
                case ClientErrorCode.InvalidOperation:
                    return true;
                case ClientErrorCode.FailedToConnect:
                case ClientErrorCode.NotConnected:
                case ClientErrorCode.SystemError:
                case ClientErrorCode.UnknownError:
                    return false;
                default:
                    throw new ArgumentOutOfRangeException(nameof(errorCode), errorCode, null);
            }
        }
    }
}
