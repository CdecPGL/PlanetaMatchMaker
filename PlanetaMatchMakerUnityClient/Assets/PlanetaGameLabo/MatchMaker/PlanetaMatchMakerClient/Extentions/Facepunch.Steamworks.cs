#if PMM_FacepunchSteamworks
using System;
using System.Threading.Tasks;
using Steamworks;

namespace PlanetaGameLabo.MatchMaker.Extentions
{
    public static class FacepunchSteamworksExtensions
    {
        /// <summary>
        /// Create room with SteamID64 by Facepunch.Steamworks.
        /// This is tested in Facepunch.Steamworks 2.3.2 .
        /// </summary>
        /// <param name="client"></param>
        /// <param name="maxPlayerCount"></param>
        /// <param name="password"></param>
        /// <exception cref="InvalidOperationException">Steam client is not ready.</exception>
        /// <returns></returns>
        public static async Task<CreateRoomResult> CreateRoomWithSteamAsync(this MatchMakerClient client,
            byte maxPlayerCount, string password = "")
        {
            if (!SteamClient.IsValid || !SteamClient.IsLoggedOn)
            {
                throw new InvalidOperationException("Steam client is invalid or not logged in.");
            }

            var steamId64 = SteamClient.SteamId.Value;
            return await client.CreateRoomWithExternalServiceAsync(maxPlayerCount,
                    GameHostConnectionEstablishMode.Steam, steamId64, password)
                .ConfigureAwait(false);
        }

        /// <summary>
        /// Join to a room on the server and get SteamNetworkingIdentity.
        /// </summary>
        /// <param name="roomId"></param>
        /// <param name="password"></param>
        /// <exception cref="ClientErrorException">Establish connection mode of the room is different from indicated one.</exception>
        /// <exception cref="ArgumentException"></exception>
        /// <returns>Game host steam networking identity</returns>
        public static async Task<SteamId> JoinRoomWithSteamAsync(this MatchMakerClient client,
            uint roomId, string password = "")
        {
            var response = await client
                .JoinRoomWithExternalServiceAsync(roomId, GameHostConnectionEstablishMode.Steam, password)
                .ConfigureAwait(false);
            var steamId64 = response.GetExternalIdAsUInt64();
            SteamId steamId = steamId64;

            return steamId;
        }
    }
}

#endif