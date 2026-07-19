using System;
using System.Threading.Tasks;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace PlanetaGameLabo.MatchMaker.Test
{
    [TestClass]
    public class RoomApiValidationTest
    {
        [TestMethod]
        public async Task OthersRoomRequiresNonEmptyP2pServicePeerId()
        {
            using (var client = new MatchMakerClient(new GameId("test")))
            {
                await Assert.ThrowsExceptionAsync<ArgumentException>(() =>
                    client.CreateRoomWithExternalServiceAsync(4,
                        GameHostConnectionEstablishMode.Others));
                await Assert.ThrowsExceptionAsync<ArgumentException>(() =>
                    client.CreateRoomWithExternalServiceAsync(4,
                        GameHostConnectionEstablishMode.Others, P2pServicePeerId.Empty));
            }
        }

        [TestMethod]
        public async Task SteamRoomRejectsClientSpecifiedP2pServicePeerId()
        {
            using (var client = new MatchMakerClient(new GameId("test")))
            {
                await Assert.ThrowsExceptionAsync<ArgumentException>(() =>
                    client.CreateRoomWithExternalServiceAsync(4,
                        GameHostConnectionEstablishMode.Steam, new P2pServicePeerId("76561198000000000")));
            }
        }
    }
}
