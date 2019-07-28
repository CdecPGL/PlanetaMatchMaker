using System;

namespace PlanetaGameLabo.MatchMaker {
    using SessionKeyType = System.UInt32;
    using VersionType = System.UInt16;

    public enum MessageErrorCode : byte {
        Ok,
        UnknownError,
        VersionMismatch,
        AuthenticationError,
        AccessDenied,
        RoomNameDuplicated,
        RoomCountReachesLimit,
        RoomDoesNotExist,
        PermissionDenied,
        JoinRejected,
        PlayerCountReachesLimit,
        RoomGroupIndexOutOfRange,
    };

    public enum MessageType : byte {
        AuthenticationRequest,
        AuthenticationReply,
        ListRoomGroupRequest,
        ListRoomGroupReply,
        CreateRoomRequest,
        CreateRoomReply,
        ListRoomRequest,
        ListRoomReply,
        JoinRoomRequest,
        JoinRoomReply,
        UpdateRoomStatusNotice,
        RandomMatchRequest
    }

    // 5 bytes. Use for notice message too
    [Serializable]
    public struct RequestMessageHeader {
        public MessageType messageType;
        public SessionKeyType sessionKey;
    }

    // 2 bytes
    [Serializable]
    public struct ReplyMessageHeader {
        public MessageType messageType;
        public MessageErrorCode errorCode;
    }

    // 2 bytes
    [Serializable]
    public struct AuthenticationRequestMessage {
        public VersionType version;
    }

    // 6 bytes
    [Serializable]
    public struct AuthenticationReplyMessage {
        public VersionType version;
        public SessionKeyType sessionKey;
    }
}