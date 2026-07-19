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
            client.HostRoomWithExternalService(GameHostConnectionEstablishMode.Steam,
                P2pServicePeerId.Empty, maxPlayerCount, password, callback);
        }

        /// <summary>
        /// Create and host new room with steam to the server.
        /// </summary>
        /// <param name="client"></param>
        /// <param name="maxPlayerCount"></param>
        /// <param name="password"></param>
        /// <returns></returns>
        public static async Task<(PlanetaMatchMakerClient.ErrorInfo errorInfo, HostRoomResult result)>
            HostRoomWithSteamAsync(this PlanetaMatchMakerClient client,
                byte maxPlayerCount, string password = "")
        {
            return await client.HostRoomWithExternalServiceAsync(GameHostConnectionEstablishMode.Steam,
                P2pServicePeerId.Empty, maxPlayerCount, password);
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
                (errorInfo, result) =>
                {
                    if (!errorInfo)
                    {
                        callback?.Invoke(errorInfo, default);
                        return;
                    }

                    if (!SteamP2pServicePeerIdParser.TryParse(result.P2pServicePeerId, out var steamId64))
                    {
                        callback?.Invoke(new PlanetaMatchMakerClient.ErrorInfo(ClientErrorCode.InvalidOperation),
                            default);
                        return;
                    }

                    SteamIdentityType steamIdentity;
                    try
                    {
                        steamIdentity = SteamLibraryHelpers.CreateSteamIdentity(steamId64);
                    }
                    catch (InvalidOperationException e)
                    {
                        Debug.LogException(e);
                        callback?.Invoke(new PlanetaMatchMakerClient.ErrorInfo(ClientErrorCode.InvalidOperation),
                            default);
                        return;
                    }

                    callback?.Invoke(errorInfo, new JoinRoomWithSteamResult(steamIdentity));
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
            var (errorInfo, result) = await client
                .JoinRoomWithExternalServiceAsync(GameHostConnectionEstablishMode.Steam, roomId, password)
                .ConfigureAwait(false);

            if (!errorInfo)
            {
                return (errorInfo, default);
            }

            if (!SteamP2pServicePeerIdParser.TryParse(result.P2pServicePeerId, out var steamId64))
            {
                return (new PlanetaMatchMakerClient.ErrorInfo(ClientErrorCode.InvalidOperation), default);
            }

            try
            {
                var steamIdentity = SteamLibraryHelpers.CreateSteamIdentity(steamId64);
                return (errorInfo, new JoinRoomWithSteamResult(steamIdentity));
            }
            catch (InvalidOperationException e)
            {
                Debug.LogException(e);
                return (new PlanetaMatchMakerClient.ErrorInfo(ClientErrorCode.InvalidOperation), default);
            }
        }
    }
}

#endif
