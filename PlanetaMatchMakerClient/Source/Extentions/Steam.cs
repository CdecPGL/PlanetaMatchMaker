#if PMM_FacepunchSteamworks || PMM_SteamworksNET
using System;
using System.Threading.Tasks;
using Steamworks;

namespace PlanetaGameLabo.MatchMaker.Extentions
{
#if PMM_FacepunchSteamworks
    using SteamIdentityType = SteamId;

    public static class SteamLibraryHelpers
    {
        /// <summary>
        /// Get SteamID64.
        /// This is tested in Facepunch.Steamworks 2.3.2 .
        /// </summary>
        /// <exception cref="InvalidOperationException">Steam client is not ready.</exception>
        /// <returns></returns>
        public static ulong GetSteamId64()
        {
            if (!SteamClient.IsValid || !SteamClient.IsLoggedOn)
            {
                throw new InvalidOperationException("Steam client is invalid or not logged in.");
            }

            return SteamClient.SteamId.Value;
        }

        /// <summary>
        /// Create SteamIdentityType from ulong.
        /// This is tested in Facepunch.Steamworks 2.3.2 .
        /// </summary>
        /// <param name="steamId64"></param>
        /// <returns></returns>
        public static SteamIdentityType CreateSteamIdentity(ulong steamId64)
        {
            return steamId64;
        }
    }
#elif PMM_SteamworksNET
    using SteamIdentityType = SteamNetworkingIdentity;

    public static class SteamLibraryHelpers
    {
        /// <summary>
        /// Get SteamID64.
        /// This is tested in Steamworks.NET 20.1.0 .
        /// </summary>
        /// <exception cref="InvalidOperationException">Steam client is not ready.</exception>
        /// <returns></returns>
        public static ulong GetSteamId64()
        {
            if (!SteamGameServerNetworkingSockets.GetIdentity(out var steamNetworkingIdentity))
            {
                throw new InvalidOperationException("Failed to get steam networking identity.");
            }
            
            return steamNetworkingIdentity.GetSteamID64();
        }

        /// <summary>
        /// Create SteamIdentityType from ulong.
        /// This is tested in Steamworks.NET 20.1.0 .
        /// </summary>
        /// <param name="steamId64"></param>
        /// <returns></returns>
        public static SteamIdentityType CreateSteamIdentity(ulong steamId64)
        {
            var steamNetworkingIdentity = new SteamNetworkingIdentity();
            steamNetworkingIdentity.SetSteamID(new CSteamID(steamId64));
            return steamNetworkingIdentity;
        }
    }
#endif

    public static class SteamExtensions
    {
        /// <summary>
        /// Create room with SteamID64.
        /// </summary>
        /// <param name="client"></param>
        /// <param name="maxPlayerCount"></param>
        /// <param name="password"></param>
        /// <exception cref="InvalidOperationException">Steam client is not ready.</exception>
        /// <returns></returns>
        public static async Task<CreateRoomResult> CreateRoomWithSteamAsync(this MatchMakerClient client,
            byte maxPlayerCount, string password = "")
        {
            var steamId64 = SteamLibraryHelpers.GetSteamId64();
            return await client.CreateRoomWithExternalServiceAsync(maxPlayerCount,
                    GameHostConnectionEstablishMode.Steam, steamId64, password)
                .ConfigureAwait(false);
        }

        /// <summary>
        /// Join to a room on the server and get SteamIdentityType.
        /// </summary>
        /// <param name="roomId"></param>
        /// <param name="password"></param>
        /// <exception cref="ClientErrorException">Establish connection mode of the room is different from indicated one.</exception>
        /// <exception cref="ArgumentException"></exception>
        /// <returns>Game host steam networking identity</returns>
        public static async Task<SteamIdentityType> JoinRoomWithSteamAsync(this MatchMakerClient client, uint roomId,
            string password = "")
        {
            var response = await client
                .JoinRoomWithExternalServiceAsync(roomId, GameHostConnectionEstablishMode.Steam, password)
                .ConfigureAwait(false);
            var steamId64 = response.GetExternalIdAsUInt64();
            var steamId = SteamLibraryHelpers.CreateSteamIdentity(steamId64);
            return steamId;
        }
    }
}

#endif