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
    }
}
