#if PMM_SteamworksNET
using System;
using System.Threading.Tasks;
using Steamworks;

namespace PlanetaGameLabo.MatchMaker.Extentions
{
    public static class SteamworksNetExtensions
    {
        /// <summary>
        /// Create room with Steamworks.NET.
        /// This is tested in Steamworks.NET 20.1.0 .
        /// </summary>
        /// <param name="client"></param>
        /// <param name="maxPlayerCount"></param>
        /// <param name="password"></param>
        /// <exception cref="InvalidOperationException">Steam client is not ready.</exception>
        /// <returns></returns>
        public static async Task<CreateRoomResult> CreateRoomWithSteamAsync(this MatchMakerClient client,
            byte maxPlayerCount, string password = "")
        {
            if (!SteamGameServerNetworkingSockets.GetIdentity(out var steamNetworkingIdentity))
            {
                throw new InvalidOperationException("Failed to get steam networking identity.");
            }

            var steamId64 = steamNetworkingIdentity.GetSteamID64();
            return await client.CreateRoomWithExternalServiceAsync(maxPlayerCount,
                    GameHostConnectionEstablishMode.Steam, steamId64, password)
                .ConfigureAwait(false);
        }

        /// <summary>
        /// Join to a room on the server and get SteamNetworkingIdentity.
        /// </summary>
        /// <param name="roomId"></param>
        /// <param name="password"></param>
        /// <exception cref="ClientErrorException"></exception>
        /// <exception cref="ArgumentException"></exception>
        /// <returns>Game host steam networking identity</returns>
        public static async Task<SteamNetworkingIdentity> JoinRoomWithSteamAsync(this MatchMakerClient client,
            uint roomId, string password = "")
        {
            var response = await client.JoinRoomAsync(roomId, password).ConfigureAwait(false);
            if (response.ConnectionEstablishMode != GameHostConnectionEstablishMode.Steam)
            {
                throw new InvalidOperationException(
                    $"Connection establish mode of the room is not steam but {response.ConnectionEstablishMode}.");
            }

            var steamId64 = response.GetExternalIdAsUInt64();
            var steamNetworkingIdentity = new SteamNetworkingIdentity();
            steamNetworkingIdentity.SetSteamID(new CSteamID(steamId64));

            return steamNetworkingIdentity;
        }
    }
}

#endif