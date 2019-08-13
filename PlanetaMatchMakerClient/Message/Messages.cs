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
        public MessageType MessageType;
        public SessionKeyType SessionKey;
    }

    // 2 bytes
    [Serializable]
    public struct ReplyMessageHeader {
        public MessageType MessageType;
        public MessageErrorCode ErrorCode;
    }

    // 2 bytes
    [Serializable]
    [Message(MessageType.AuthenticationRequest)]
    public struct AuthenticationRequestMessage {
        public VersionType Version;
    }

    // 6 bytes
    [Serializable]
    [Message(MessageType.AuthenticationReply)]
    public struct AuthenticationReplyMessage {
        public VersionType Version;
        public SessionKeyType SessionKey;
    }

    // 1 bytes
    [Serializable]
    [Message(MessageType.ListRoomGroupRequest)]
    public struct ListRoomGroupRequestMessage {
        public byte Dummy;
    }

    // 241 bytes
    [Serializable]
    [Message(MessageType.ListRoomGroupReply)]
    public struct ListRoomGroupReplyMessage {
        [Serializable]
        public struct RoomGroupInfo {
            [FixedLength(ClientConstants.RoomGroupNameLength)]
            public string Name;
        }

        public byte RoomGroupCount;

        [FixedLength(ClientConstants.RoomGroupMaxCount)]
        public RoomGroupInfo[] RoomGroupInfoList;
    }

    // 43 bytes
    [Serializable]
    [Message(MessageType.CreateRoomRequest)]
    public struct CreateRoomRequestMessage {
        public RoomGroupIndexType GroupIndex;

        [FixedLength(ClientConstants.RoomNameLength)]
        public string Name;

        public RoomFlag Flags;

        [FixedLength(ClientConstants.RoomPasswordLength)]
        public string Password;

        public byte MaxPlayerCount;
    }

    // 4 bytes
    [Serializable]
    [Message(MessageType.CreateRoomReply)]
    public struct CreateRoomReplyMessage {
        public RoomIdType RoomId;
    }

    // 5 bytes
    [Serializable]
    [Message(MessageType.ListRoomRequest)]
    public struct ListRoomRequestMessage {
        public RoomGroupIndexType GroupIndex;
        public byte StartIndex;
        public byte EndIndex;
        public RoomDataSortKind SortKind;
        public byte Flags; //filter conditions about room
    }

    // 238 bytes
    [Serializable]
    [Message(MessageType.ListRoomReply)]
    public struct ListRoomReplyMessage {
        //39 bytes
        [Serializable]
        public struct RoomInfo {
            public RoomIdType RoomId;

            [FixedLength(ClientConstants.RoomNameLength)]
            public string Name;

            public RoomFlag Flags;
            public byte MaxPlayerCount;
            public byte CurrentPlayerCount;
            public Datetime CreateDatetime;
        }

        public byte TotalRoomCount; // the number of rooms server managing
        public byte ResultRoomCount; // the number of rooms for request
        public byte ReplyRoomStartIndex; // the index of start room in this message
        public byte ReplyRoomEndIndex; // the index of end room in this message

        [FixedLength(ClientConstants.ListRoomReplyRoomInfoCount)]
        public RoomInfo[] RoomInfoList;
    }

    // 21 bytes
    [Serializable]
    [Message(MessageType.JoinRoomRequest)]
    public struct JoinRoomRequestMessage {
        public RoomGroupIndexType GroupIndex;
        public RoomIdType RoomId;

        [FixedLength(ClientConstants.RoomPasswordLength)]
        public string Password;
    }

    //18 bytes
    [Serializable]
    [Message(MessageType.JoinRoomReply)]
    public struct JoinRoomReplyMessage {
        public ClientAddress HostAddress;
    }

    // 6 bytes
    [Serializable]
    [Message(MessageType.UpdateRoomStatusNotice)]
    public struct UpdateRoomStatusNoticeMessage {
        public enum RoomStatus : byte {
            Open,
            Close,
            Remove
        }

        public RoomGroupIndexType GroupIndex;
        public RoomIdType RoomId;
        public RoomStatus Status;
    }
}