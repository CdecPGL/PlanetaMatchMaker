using System;
using CdecPGL.MinimalSerializer;

// Ignore below suggestion because we use .NET type only for type alias because type alias is not available for type keyword in .Net Standard 2.0
// ReSharper disable BuiltInTypeReferenceStyle

namespace PlanetaGameLabo.MatchMaker
{
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

        // The operation is invalid in the current state.
        OperationInvalid,

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

        // The number of room exceeds limit.
        RoomCountExceedsLimit,

        // Connection establish mode of the room host doesn't match expected one in the client.
        RoomConnectionEstablishModeMismatch,

        // Request is failed because the client is already hosting room.
        ClientAlreadyHostingRoom,
    };

    internal enum MessageType : byte
    {
        Authentication,
        CreateRoom,
        ListRoom,
        JoinRoom,
        UpdateRoomStatus,
        ConnectionTest,
        RandomMatch,
        KeepAlive
    }

    // 1 bytes. Use for notice message too
    [Serializable]
    internal struct RequestMessageHeader
    {
        public MessageType MessageType;
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
    [Message(MessageType.Authentication)]
    internal struct AuthenticationRequestMessage
    {
        public VersionType Version;

        [FixedLength(ClientConstants.PlayerNameLength)]
        public string PlayerName;
    }

    // 4 bytes
    [Serializable]
    [Message(MessageType.Authentication)]
    internal struct AuthenticationReplyMessage
    {
        public VersionType Version;
        public ushort PlayerTag;
    }

    // 84 bytes
    [Serializable]
    [Message(MessageType.CreateRoom)]
    internal struct CreateRoomRequestMessage
    {
        [FixedLength(RoomConstants.RoomPasswordLength)]
        public string Password;

        public byte MaxPlayerCount;

        public GameHostConnectionEstablishMode ConnectionEstablishMode;

        public ushort PortNumber;

        [FixedLength(RoomConstants.GameHostExternalIdLength)]
        public byte[] ExternalId;
    }

    // 4 bytes
    [Serializable]
    [Message(MessageType.CreateRoom)]
    internal struct CreateRoomReplyMessage
    {
        public RoomIdType RoomId;
    }

    // 30 bytes
    [Serializable]
    [Message(MessageType.ListRoom)]
    internal struct ListRoomRequestMessage
    {
        public UInt16 StartIndex;
        public UInt16 Count;
        public RoomDataSortKind SortKind;
        public RoomSearchTargetFlag SearchTargetFlags;
        public PlayerFullName SearchFullName;
    }

    // 246 bytes
    [Serializable]
    [Message(MessageType.ListRoom)]
    internal struct ListRoomReplyMessage
    {
        //40 bytes
        [Serializable]
        public struct RoomInfo
        {
            public RoomIdType RoomId;
            public PlayerFullName HostPlayerFullName;
            public RoomSettingFlag SettingFlags;
            public byte MaxPlayerCount;
            public byte CurrentPlayerCount;
            public Datetime CreateDatetime;
            public GameHostConnectionEstablishMode ConnectionEstablishMode;
        }

        public UInt16 TotalRoomCount; // the number of rooms server managing
        public UInt16 MatchedRoomCount; // the number of rooms matched to requested condition
        public UInt16 ReplyRoomCount; // the number of rooms in these replies

        [FixedLength(RoomConstants.ListRoomReplyRoomInfoCount)]
        public RoomInfo[] RoomInfoList;
    }

    // 21 bytes
    [Serializable]
    [Message(MessageType.JoinRoom)]
    internal struct JoinRoomRequestMessage
    {
        public RoomIdType RoomId;

        public GameHostConnectionEstablishMode ConnectionEstablishMode;

        [FixedLength(RoomConstants.RoomPasswordLength)]
        public string Password;
    }

    // 82 bytes
    [Serializable]
    [Message(MessageType.JoinRoom)]
    internal struct JoinRoomReplyMessage
    {
        public EndPoint GameHostEndPoint;

        [FixedLength(RoomConstants.GameHostExternalIdLength)]
        public byte[] GameHostExternalId;
    }

    // 7 bytes
    [Serializable]
    [Message(MessageType.UpdateRoomStatus)]
    internal struct UpdateRoomStatusNoticeMessage
    {
        public RoomIdType RoomId;
        public RoomStatus Status;
        public bool IsCurrentPlayerCountChanged;
        public byte CurrentPlayerCount;
    }

    // 21 bytes
    [Serializable]
    [Message(MessageType.ConnectionTest)]
    internal struct ConnectionTestRequestMessage
    {
        public TransportProtocol Protocol;
        public ushort PortNumber;
    }

    //18 bytes
    [Serializable]
    [Message(MessageType.ConnectionTest)]
    internal struct ConnectionTestReplyMessage
    {
        public bool Succeed;
    }

    // 1 byte
    [Serializable]
    [Message(MessageType.KeepAlive)]
    internal struct KeepAliveNoticeMessage
    {
        public byte Dummy;
    }

#pragma warning restore 0649
}
