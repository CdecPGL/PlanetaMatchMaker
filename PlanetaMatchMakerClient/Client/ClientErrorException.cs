﻿using System;

namespace PlanetaGameLabo.MatchMaker {
    public sealed class ClientErrorException : Exception {
        public ClientErrorCode clientErrorCode { get; }

        public string extraMessage { get; }

        public string message { get; }

        public ClientErrorException(ClientErrorCode error_code) {
            clientErrorCode = error_code;
            message = GenerateMessage();
        }

        public ClientErrorException(ClientErrorCode error_code, string extra_message) {
            clientErrorCode = error_code;
            extraMessage = extra_message;
            message = GenerateMessage();
        }

        private string GenerateMessage() {
            if (string.IsNullOrWhiteSpace(extraMessage)) {
                return clientErrorCode.GetClientErrorMessage();
            }

            return clientErrorCode.GetClientErrorMessage() + ": " + extraMessage;
        }
    }

    public enum ClientErrorCode {
        Ok,
        MessageSendError,
        MessageReceptionError,
        RequestError,
    };

    public static class ClientErrorCodeExtensions {
        public static string GetClientErrorMessage(this ClientErrorCode error_code) {
            switch (error_code) {
                case ClientErrorCode.Ok:
                    return "Ok";
                case ClientErrorCode.MessageSendError:
                    return "Failed to send messages.";
                case ClientErrorCode.MessageReceptionError:
                    return "Failed to receive messages.";
                case ClientErrorCode.RequestError:
                    return "Request doesn't processed correctly on the server.";
                default:
                    return "";
            }
        }
    }
}