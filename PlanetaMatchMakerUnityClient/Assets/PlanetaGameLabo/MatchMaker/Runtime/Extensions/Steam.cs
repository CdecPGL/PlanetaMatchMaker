#if PMM_FacepunchSteamworks || PMM_SteamworksNET
using System;
using System.Threading.Tasks;
using Steamworks;
using UnityEngine;

namespace PlanetaGameLabo.MatchMaker.Extentions
{
#if PMM_FacepunchSteamworks
    using SteamIdentityType = SteamId;

#elif PMM_SteamworksNET
    using SteamIdentityType = SteamNetworkingIdentity;
#endif

    public readonly struct JoinRoomWithSteamResult
    {
        public JoinRoomWithSteamResult(SteamIdentityType steamIdentity)
        {
            this.steamIdentity = steamIdentity;
        }

        public readonly SteamIdentityType steamIdentity;
    }

    public static class SteamExtensionsForUnity
    {
        /// <summary>
        /// Create and host new room with steam to the server.
        /// </summary>
        /// <param name="client"></param>
        /// <param name="maxPlayerCount"></param>
        /// <param name="password"></param>
        /// <param name="callback"></param>
        /// <returns></returns>
        public static void HostRoomWithSteam(this PlanetaMatchMakerClient client, byte maxPlayerCount,
            string password = "",
            Action<PlanetaMatchMakerClient.ErrorInfo, HostRoomResult> callback = null)
        {
            var steamId64 = SteamLibraryHelpers.GetSteamId64();
            client.HostRoomWithExternalService(GameHostConnectionEstablishMode.Steam, steamId64, maxPlayerCount,
                password, callback);
        }

        /// <summary>
        /// Create and host new room with steam to the server.
        /// </summary>
        /// <param name="client"></param>
        /// <param name="maxPlayerCount"></param>
        /// <param name="password"></param>
        /// <exception cref="InvalidOperationException">Steam client is not ready.</exception>
        /// <returns></returns>
        public static async Task<(PlanetaMatchMakerClient.ErrorInfo errorInfo, HostRoomResult result)>
            HostRoomWithSteamAsync(this PlanetaMatchMakerClient client,
                byte maxPlayerCount, string password = "")
        {
	        ulong steamId64;
	        try {
		        steamId64 = SteamLibraryHelpers.GetSteamId64();
	        }
	        catch (InvalidOperationException e) {
		        Debug.LogException(e);
		        return (new PlanetaMatchMakerClient.ErrorInfo(ClientErrorCode.InvalidOperation), default);
	        }
	        
	        return await client.HostRoomWithExternalServiceAsync(GameHostConnectionEstablishMode.Steam, steamId64,
			        maxPlayerCount, password);
	        }

        /// <summary>
        /// Join to a room on the server and get SteamIdentity.
        /// </summary>
        /// <param name="client"></param>
        /// <param name="roomId"></param>
        /// <param name="password"></param>
        /// <param name="callback"></param>
        /// <exception cref="ClientErrorException">Establish connection mode of the room is different from indicated one.</exception>
        /// <exception cref="ArgumentException"></exception>
        /// <returns>Game host steam networking identity</returns>
        public static void JoinRoomWithSteam(this PlanetaMatchMakerClient client, uint roomId, string password = "",
            Action<PlanetaMatchMakerClient.ErrorInfo, JoinRoomWithSteamResult> callback = null)
        {
            client.JoinRoomWithExternalService(GameHostConnectionEstablishMode.Steam, roomId, password,
                (error, result) =>
                {
                    var steamId64 = result.GetExternalIdAsUInt64();
                    var steamIdentity = SteamLibraryHelpers.CreateSteamIdentity(steamId64);
                    callback?.Invoke(error, new JoinRoomWithSteamResult(steamIdentity));
                });
        }

        /// <summary>
        /// Join to a room on the server and get SteamIdentity.
        /// </summary>
        /// <param name="client"></param>
        /// <param name="roomId"></param>
        /// <param name="password"></param>
        /// <exception cref="ClientErrorException">Establish connection mode of the room is different from indicated one.</exception>
        /// <exception cref="ArgumentException"></exception>
        /// <returns>Game host steam networking identity</returns>
        public static async
            Task<(PlanetaMatchMakerClient.ErrorInfo errorInfo, JoinRoomWithSteamResult result)>
            JoinRoomWithSteamAsync(this PlanetaMatchMakerClient client, uint roomId, string password = "")
        {
            var (error, result) = await client
                .JoinRoomWithExternalServiceAsync(GameHostConnectionEstablishMode.Steam, roomId, password)
                .ConfigureAwait(false);

            try {
	            var steamId64 = result.GetExternalIdAsUInt64();
	            var steamIdentity = SteamLibraryHelpers.CreateSteamIdentity(steamId64);
	            return (error, new JoinRoomWithSteamResult(steamIdentity));
            }
            catch (InvalidOperationException e) {
	            Debug.LogException(e);
	            return (new PlanetaMatchMakerClient.ErrorInfo(ClientErrorCode.InvalidOperation), default);
            }
        }
    }
}

#endif