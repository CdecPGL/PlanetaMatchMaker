using System;
using System.Linq;
using CdecPGL.MinimalSerializer;

namespace PlanetaGameLabo.MatchMaker
{
    using SessionKeyType = UInt32;
    using VersionType = UInt16;
    using RoomIdType = UInt32;

#pragma warning disable 0649

    internal enum MessageErrorCode : byte
    {
        Ok,

        // Server internal error.
        ServerError,

        // Server api version and client api version are not same.
        ApiVersionMismatch,

        // Wrong parameters which must be rejected in the client is passed for request.
        RequestParameterWrong,

        // Indicated room is not found.
        RoomNotFound,

        // Indicated password of room is not correct.
        RoomPasswordWrong,

        // The number of player reaches limit.
        RoomFull,

        // Request is rejected because indicated room is the room which you are not host of or closed.
        RoomPermissionDenied,

        // The number of room reaches limit.
        RoomGroupFull,

        // Request is failed because the client is already hosting room.
        ClientAlreadyHostingRoom,
    };

    internal enum MessageType : byte
    {
        AuthenticationRequest,
        AuthenticationReply,
        CreateRoomRequest,
        CreateRoomReply,
        ListRoomRequest,
        ListRoomReply,
        JoinRoomRequest,
        JoinRoomReply,
        UpdateRoomStatusNotice,
        ConnectionTestRequest,
        ConnectionTestReply,
        RandomMatchRequest,
        KeepAliveNotice
    }

    // 5 bytes. Use for notice message too
    [Serializable]
    internal struct RequestMessageHeader
    {
        public MessageType MessageType;
        public SessionKeyType SessionKey;
    }

    // 2 bytes
    [Serializable]
    internal struct ReplyMessageHeader
    {
        public MessageType MessageType;
        public MessageErrorCode ErrorCode;
    }

    // 2 bytes
    [Serializable]
    [Message(MessageType.AuthenticationRequest)]
    internal struct AuthenticationRequestMessage
    {
        public VersionType Version;

        [FixedLength(ClientConstants.PlayerNameLength)]
        public string PlayerName;
    }

    // 6 bytes
    [Serializable]
    [Message(MessageType.AuthenticationReply)]
    internal struct AuthenticationReplyMessage
    {
        public VersionType Version;
        public SessionKeyType SessionKey;
        public ushort PlayerTag;
    }

    // 148 bytes
    [Serializable]
    [Message(MessageType.CreateRoomRequest)]
    internal struct CreateRoomRequestMessage
    {
        [FixedLength(RoomConstants.RoomPasswordLength)]
        public string Password;

        public byte MaxPlayerCount;

        public RoomSignalingMethod SignalingMethod;

        public ushort PortNumber;

        [FixedLength(RoomConstants.RoomGameHostExternalIdLength)]
        public byte[] ExternalId;
    }

    // 4 bytes
    [Serializable]
    [Message(MessageType.CreateRoomReply)]
    internal struct CreateRoomReplyMessage
    {
        public RoomIdType RoomId;
    }

    // 32 bytes
    [Serializable]
    [Message(MessageType.ListRoomRequest)]
    internal struct ListRoomRequestMessage
    {
        public UInt16 StartIndex;
        public UInt16 Count;
        public RoomDataSortKind SortKind;
        public RoomSearchTargetFlag SearchTargetFlags;
        public PlayerFullName SearchFullName;
    }

    // 252 bytes
    [Serializable]
    [Message(MessageType.ListRoomReply)]
    internal struct ListRoomReplyMessage
    {
        //39 bytes
        [Serializable]
        public struct RoomInfo
        {
            public RoomIdType RoomId;
            public PlayerFullName HostPlayerFullName;
            public RoomSettingFlag SettingFlags;
            public byte MaxPlayerCount;
            public byte CurrentPlayerCount;
            public Datetime CreateDatetime;
        }

        public UInt16 TotalRoomCount; // the number of rooms server managing
        public UInt16 MatchedRoomCount; // the number of rooms matched to requested condition
        public UInt16 ReplyRoomCount; // the number of rooms in these replies

        [FixedLength(RoomConstants.ListRoomReplyRoomInfoCount)]
        public RoomInfo[] RoomInfoList;
    }

    // 20 bytes
    [Serializable]
    [Message(MessageType.JoinRoomRequest)]
    internal struct JoinRoomRequestMessage
    {
        public RoomIdType RoomId;

        [FixedLength(RoomConstants.RoomPasswordLength)]
        public string Password;
    }

    // 147 bytes
    [Serializable]
    [Message(MessageType.JoinRoomReply)]
    internal struct JoinRoomReplyMessage
    {
        public RoomSignalingMethod GameHostSignalingMethod;
        public EndPoint GameHostEndPoint;
        [FixedLength(RoomConstants.RoomGameHostExternalIdLength)]
        public byte[] GameHostExternalId;
    }

    // 7 bytes
    [Serializable]
    [Message(MessageType.UpdateRoomStatusNotice)]
    internal struct UpdateRoomStatusNoticeMessage
    {
        public RoomIdType RoomId;
        public RoomStatus Status;
        public bool IsCurrentPlayerCountChanged;
        public byte CurrentPlayerCount;
    }

    // 21 bytes
    [Serializable]
    [Message(MessageType.ConnectionTestRequest)]
    internal struct ConnectionTestRequestMessage
    {
        public TransportProtocol Protocol;
        public ushort PortNumber;
    }

    //18 bytes
    [Serializable]
    [Message(MessageType.ConnectionTestReply)]
    internal struct ConnectionTestReplyMessage
    {
        public bool Succeed;
    }

    // 1 byte
    [Serializable]
    [Message(MessageType.KeepAliveNotice)]
    internal struct KeepAliveNoticeMessage
    {
        public byte Dummy;
    }

#pragma warning restore 0649
}