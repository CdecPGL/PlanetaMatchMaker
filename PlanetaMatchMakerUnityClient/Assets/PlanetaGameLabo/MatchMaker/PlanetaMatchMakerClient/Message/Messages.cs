using System;
using CdecPGL.MinimalSerializer;

namespace PlanetaGameLabo.MatchMaker
{
    using SessionKeyType = UInt32;
    using VersionType = UInt16;
    using RoomGroupIndexType = Byte;
    using RoomIdType = UInt32;

#pragma warning disable 0649

    internal enum MessageErrorCode : byte
    {
        Ok,
        UnknownError,
        ApiVersionMismatch,
        AuthenticationError,
        AccessDenied,
        RoomNameDuplicated,
        RoomCountReachesLimit,
        RoomDoesNotExist,
        RoomIsNotOpened,
        RoomPasswordIsWrong,
        RoomNameIsEmpty,
        PermissionDenied,
        JoinRejected,
        PlayerCountReachesLimit,
        RoomGroupIndexOutOfRange,
    };

    internal enum MessageType : byte
    {
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
        ConnectionTestRequest,
        ConnectionTestReply,
        RandomMatchRequest
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

    // 1 bytes
    [Serializable]
    [Message(MessageType.ListRoomGroupRequest)]
    internal struct ListRoomGroupRequestMessage
    {
        public byte Dummy;
    }

    // 245 bytes
    [Serializable]
    [Message(MessageType.ListRoomGroupReply)]
    internal struct ListRoomGroupReplyMessage
    {
        [Serializable]
        public struct RoomGroupInfo
        {
            [FixedLength(RoomConstants.RoomGroupNameLength)]
            public string Name;
        }

        public byte RoomGroupCount;
        public uint MaxRoomCountPerRoomGroup;

        [FixedLength(RoomConstants.RoomGroupMaxCount)]
        public RoomGroupInfo[] RoomGroupInfoList;
    }

    // 21 bytes
    [Serializable]
    [Message(MessageType.CreateRoomRequest)]
    internal struct CreateRoomRequestMessage
    {
        public RoomGroupIndexType GroupIndex;

        public bool IsPublic;

        [FixedLength(RoomConstants.RoomPasswordLength)]
        public string Password;

        public byte MaxPlayerCount;

        public ushort portNumber;
    }

    // 4 bytes
    [Serializable]
    [Message(MessageType.CreateRoomReply)]
    internal struct CreateRoomReplyMessage
    {
        public RoomIdType RoomId;
    }

    // 31 bytes
    [Serializable]
    [Message(MessageType.ListRoomRequest)]
    internal struct ListRoomRequestMessage
    {
        public RoomGroupIndexType GroupIndex;
        public byte StartIndex;
        public byte Count;
        public RoomDataSortKind SortKind;
        public RoomSearchTargetFlag SearchTargetFlags;
        public PlayerFullName SearchFullName;
    }

    // 249 bytes
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

        public byte TotalRoomCount; // the number of rooms server managing
        public byte MatchedRoomCount; // the number of rooms matched to requested condition
        public byte ReplyRoomCount; // the number of rooms in these replies

        [FixedLength(RoomConstants.ListRoomReplyRoomInfoCount)]
        public RoomInfo[] RoomInfoList;
    }

    // 21 bytes
    [Serializable]
    [Message(MessageType.JoinRoomRequest)]
    internal struct JoinRoomRequestMessage
    {
        public RoomGroupIndexType GroupIndex;
        public RoomIdType RoomId;

        [FixedLength(RoomConstants.RoomPasswordLength)]
        public string Password;
    }

    //18 bytes
    [Serializable]
    [Message(MessageType.JoinRoomReply)]
    internal struct JoinRoomReplyMessage
    {
        public EndPoint GameHostEndPoint;
    }

    // 8 bytes
    [Serializable]
    [Message(MessageType.UpdateRoomStatusNotice)]
    internal struct UpdateRoomStatusNoticeMessage
    {
        public RoomGroupIndexType GroupIndex;
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

#pragma warning restore 0649
}