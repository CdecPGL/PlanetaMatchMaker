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
    }
}
