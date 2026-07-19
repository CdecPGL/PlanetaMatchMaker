using System;
using CdecPGL.MinimalSerializer;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace PlanetaGameLabo.MatchMaker.Test
{
    [TestClass]
    public class MessageProtocolTest
    {
        [TestMethod]
        public void ListRoomReplyRecordFitsProtocolLimit()
        {
            const int maxRecordSize = 256;
            Assert.AreEqual(5, RoomConstants.ListRoomReplyRoomInfoCount);
            Assert.AreEqual(216, Serializer.GetSerializedSize<ListRoomReplyMessage>());
            Assert.IsTrue(Serializer.GetSerializedSize<ReplyMessageHeader>() +
                          Serializer.GetSerializedSize<ListRoomReplyMessage>() <= maxRecordSize);
        }

        [TestMethod]
        public void CreateAndJoinRoomRecordsFitProtocolLimit()
        {
            const int maxRecordSize = 256;
            Assert.AreEqual(128, RoomConstants.P2pServicePeerIdLength);
            Assert.AreEqual(148, Serializer.GetSerializedSize<CreateRoomRequestMessage>());
            Assert.AreEqual(146, Serializer.GetSerializedSize<JoinRoomReplyMessage>());
            Assert.AreEqual(153, Serializer.GetSerializedSize<RequestMessageHeader>() +
                                 Serializer.GetSerializedSize<CreateRoomRequestMessage>());
            Assert.AreEqual(152, Serializer.GetSerializedSize<ReplyMessageHeader>() +
                                 Serializer.GetSerializedSize<JoinRoomReplyMessage>());
            Assert.IsTrue(Serializer.GetSerializedSize<RequestMessageHeader>() +
                          Serializer.GetSerializedSize<CreateRoomRequestMessage>() <= maxRecordSize);
            Assert.IsTrue(Serializer.GetSerializedSize<ReplyMessageHeader>() +
                          Serializer.GetSerializedSize<JoinRoomReplyMessage>() <= maxRecordSize);
        }

        [TestMethod]
        public void JoinRoomReplyPreservesP2pServicePeerId()
        {
            var expected = new string('p', RoomConstants.P2pServicePeerIdLength);
            var serialized = Serializer.Serialize(new JoinRoomReplyMessage
            {
                GameHostEndPoint = new EndPoint { IpAddress = new byte[16] },
                GameHostP2pServicePeerId = expected
            });

            var deserialized = Serializer.Deserialize<JoinRoomReplyMessage>(serialized);

            Assert.AreEqual(expected, deserialized.GameHostP2pServicePeerId);
        }

        [TestMethod]
        public void MessageErrorCodesHaveStableWireValues()
        {
            Assert.AreEqual((byte)0, (byte)MessageErrorCode.Ok);
            Assert.AreEqual((byte)1, (byte)MessageErrorCode.ServerError);
            Assert.AreEqual((byte)2, (byte)MessageErrorCode.OperationInvalid);
            Assert.AreEqual((byte)3, (byte)MessageErrorCode.RequestParameterWrong);
            Assert.AreEqual((byte)4, (byte)MessageErrorCode.RoomNotFound);
            Assert.AreEqual((byte)5, (byte)MessageErrorCode.RoomPasswordWrong);
            Assert.AreEqual((byte)6, (byte)MessageErrorCode.RoomFull);
            Assert.AreEqual((byte)7, (byte)MessageErrorCode.RoomPermissionDenied);
            Assert.AreEqual((byte)8, (byte)MessageErrorCode.RoomCountExceedsLimit);
            Assert.AreEqual((byte)9, (byte)MessageErrorCode.RoomConnectionEstablishModeMismatch);
            Assert.AreEqual((byte)10, (byte)MessageErrorCode.ClientAlreadyHostingRoom);
        }

        [TestMethod]
        public void AuthenticationMethodsHaveStableWireValues()
        {
            Assert.AreEqual((byte)0, (byte)AuthenticationMethod.None);
            Assert.AreEqual((byte)1, (byte)AuthenticationMethod.Steam);
            Assert.IsFalse(Enum.IsDefined(typeof(AuthenticationMethod), (byte)2));
        }

        [TestMethod]
        public void AuthenticationResultsHaveStableWireValues()
        {
            Assert.AreEqual((byte)0, (byte)AuthenticationResult.Success);
            Assert.AreEqual((byte)1, (byte)AuthenticationResult.ApiVersionMismatch);
            Assert.AreEqual((byte)2, (byte)AuthenticationResult.GameIdMismatch);
            Assert.AreEqual((byte)3, (byte)AuthenticationResult.GameVersionMismatch);
            Assert.AreEqual((byte)4, (byte)AuthenticationResult.UnsupportedAuthenticationMethod);
            Assert.AreEqual((byte)5, (byte)AuthenticationResult.AuthenticationDataFormatInvalid);
            Assert.AreEqual((byte)6, (byte)AuthenticationResult.AuthenticationDataSizeExceeded);
            Assert.AreEqual((byte)7, (byte)AuthenticationResult.AuthenticationDataInvalid);
            Assert.AreEqual((byte)8, (byte)AuthenticationResult.InsecureConnection);
            Assert.AreEqual((byte)9, (byte)AuthenticationResult.SteamTicketInvalid);
            Assert.AreEqual((byte)10, (byte)AuthenticationResult.SteamOwnershipCheckFailed);
            Assert.AreEqual((byte)11, (byte)AuthenticationResult.SteamAuthenticationServiceUnavailable);
            Assert.IsFalse(Enum.IsDefined(typeof(AuthenticationResult), (byte)12));
        }
    }
}
