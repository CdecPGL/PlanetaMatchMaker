using System;
using PlanetaGameLabo.Serializer;

namespace PlanetaGameLabo.MatchMaker {
    using SessionKeyType = UInt32;
    using VersionType = UInt16;
    using RoomGroupIndexType = Byte;
    using RoomIdType = UInt32;

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

    [Serializable]
    [Flags]
    public enum RoomFlag : byte {
        IsPrivate = 1,
        IsOpen = 2
    }

    [Serializable]
    public enum RoomDataSortKind : byte {
        NameAscending,
        NameDescending,
        CreateDatetimeAscending,
        CreateDatetimeDescending
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
    [Message(MessageType.AuthenticationRequest)]
    public struct AuthenticationRequestMessage {
        public VersionType version;
    }

    // 6 bytes
    [Serializable]
    [Message(MessageType.AuthenticationReply)]
    public struct AuthenticationReplyMessage {
        public VersionType version;
        public SessionKeyType sessionKey;
    }

    // 1 bytes
    [Serializable]
    [Message(MessageType.ListRoomGroupRequest)]
    public struct ListRoomGroupRequestMessage {
        public byte dummy;
    }

    // 241 bytes
    [Serializable]
    [Message(MessageType.ListRoomGroupReply)]
    public struct ListRoomGroupReplyMessage {
        public struct RoomGroupInfo {
            [FixedLength(ClientConstants.roomGroupNameLength)]
            public string name;
        }

        public byte roomGroupCount;

        [FixedLength(ClientConstants.roomGroupMaxCount)]
        public RoomGroupInfo[] roomGroupInfoList;
    }

    // 43 bytes
    [Serializable]
    [Message(MessageType.CreateRoomRequest)]
    public struct CreateRoomRequestMessage {
        public RoomGroupIndexType groupIndex;

        [FixedLength(ClientConstants.roomNameLength)]
        public string name;

        public RoomFlag flags;

        [FixedLength(ClientConstants.roomPasswordLength)]
        public string password;

        public byte maxPlayerCount;
    }

    // 4 bytes
    [Serializable]
    [Message(MessageType.CreateRoomReply)]
    public struct CreateRoomReplyMessage {
        public RoomIdType roomId;
    }

    // 5 bytes
    [Serializable]
    [Message(MessageType.ListRoomRequest)]
    public struct ListRoomRequestMessage {
        public RoomGroupIndexType groupIndex;
        public byte startIndex;
        public byte endIndex;
        public RoomDataSortKind sortKind;
        public byte flags; //filter conditions about room
    }

    // 238 bytes
    [Serializable]
    [Message(MessageType.ListRoomReply)]
    public struct ListRoomReplyMessage {
        //39 bytes
        public struct RoomInfo {
            public RoomIdType roomId;

            [FixedLength(ClientConstants.roomNameLength)]
            public string name;

            public RoomFlag flags;
            public byte maxPlayerCount;
            public byte currentPlayerCount;
            public Datetime createDatetime;
        }

        public byte totalRoomCount; // the number of rooms server managing
        public byte resultRoomCount; // the number of rooms for request
        public byte replyRoomStartIndex; // the index of start room in this message
        public byte replyRoomEndIndex; // the index of end room in this message

        [FixedLength(ClientConstants.listRoomReplyRoomInfoCount)]
        public RoomInfo[] roomInfoList;
    }

    // 21 bytes
    [Serializable]
    [Message(MessageType.JoinRoomRequest)]
    public struct JoinRoomRequestMessage {
        public RoomGroupIndexType groupIndex;
        public RoomIdType roomId;

        [FixedLength(ClientConstants.roomPasswordLength)]
        public string password;
    }

    //18 bytes
    [Serializable]
    [Message(MessageType.JoinRoomReply)]
    public struct JoinRoomReplyMessage {
        public ClientAddress hostAddress;
    }

    // 6 bytes
    [Serializable]
    [Message(MessageType.UpdateRoomStatusNotice)]
    public struct UpdateRoomStatusNoticeMessage {
        public enum Status : byte {
            Open,
            Close,
            Remove
        }

        public RoomGroupIndexType groupIndex;
        public RoomIdType roomId;
        public Status status;
    }
}