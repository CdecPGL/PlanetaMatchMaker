using System;
using System.Threading.Tasks;
using UnityEngine;

namespace PlanetaGameLabo.MatchMaker
{
    [AddComponentMenu("PlanetaGameLabo/MatchMaker/PlanetaMatchMakerClient"), DisallowMultipleComponent]
    public sealed class PlanetaMatchMakerClient : MonoBehaviour, IDisposable
    {
        /// <summary>
        /// true if connected to the server.
        /// </summary>
        public bool connected => _client?.Connected ?? false;

        /// <summary>
        /// true if this client hosting a room.
        /// </summary>
        public bool isHostingRoom => _client?.IsHostingRoom ?? false;

        public string serverAddress
        {
            get => _serverAddress;
            set => _serverAddress = value;
        }

        public ushort serverPort
        {
            get => _serverPort;
            set => _serverPort = value;
        }

        /// <summary>
        /// Connect to matching server.
        /// </summary>
        /// <exception cref="ClientErrorException"></exception>
        /// <returns></returns>
        public async Task ConnectAsync()
        {
            try
            {
                await Task.Run(() => _client.ConnectAsync(_serverAddress, _serverPort));
            }
            catch (Exception e)
            {
                Debug.LogException(e);
                throw;
            }
        }

        /// <summary>
        /// Close the connection.
        /// </summary>
        public void Close()
        {
            _client.Close();
        }

        /// <summary>
        /// Get a room group list from the server.
        /// </summary>
        /// <exception cref="ClientErrorException"></exception>
        /// <returns></returns>
        public async Task<RoomGroupResult[]> GetRoomGroupListAsync()
        {
            try
            {
                return await Task.Run(() => _client.GetRoomGroupListAsync());
            }
            catch (Exception e)
            {
                Debug.LogException(e);
                throw;
            }
        }

        /// <summary>
        /// Create and host new room to the server.
        /// </summary>
        /// <param name="roomGroupIndex"></param>
        /// <param name="roomName"></param>
        /// <exception cref="ClientErrorException"></exception>
        /// <returns></returns>
        public async Task CreateRoomAsync(byte roomGroupIndex, string roomName, byte maxPlayerCount)
        {
            try
            {
                await Task.Run(() => _client.CreateRoomAsync(roomGroupIndex, roomName, maxPlayerCount));
            }
            catch (Exception e)
            {
                Debug.LogException(e);
                throw;
            }
        }

        /// <summary>
        /// Get a room list from the server.
        /// </summary>
        /// <param name="roomGroupIndex"></param>
        /// <param name="startIndex"></param>
        /// <param name="count"></param>
        /// <param name="sortKind"></param>
        /// <param name="flags"></param>
        /// <exception cref="ClientErrorException"></exception>
        /// <returns></returns>
        public async Task<(int totalRoomCount, RoomResult[] roomInfoList)> GetRoomListAsync(
            byte roomGroupIndex, byte startIndex,
            byte count, RoomDataSortKind sortKind, byte flags)
        {
            try
            {
                return await Task.Run(
                    () => _client.GetRoomListAsync(roomGroupIndex, startIndex, count, sortKind, flags));
            }
            catch (Exception e)
            {
                Debug.LogException(e);
                throw;
            }
        }

        /// <summary>
        /// Join to a room on the server.
        /// </summary>
        /// <param name="roomGroupIndex"></param>
        /// <param name="roomId"></param>
        /// <exception cref="ClientErrorException"></exception>
        /// <returns></returns>
        public async Task<ClientAddress> JoinRoomAsync(byte roomGroupIndex, uint roomId)
        {
            try
            {
                return await Task.Run(
                    () => _client.JoinRoomAsync(roomGroupIndex, roomId));
            }
            catch (Exception e)
            {
                Debug.LogException(e);
                throw;
            }
        }

        /// <summary>
        /// Update hosting room status on the server.
        /// </summary>
        /// <param name="roomStatus"></param>
        /// <exception cref="ClientErrorException"></exception>
        /// <returns></returns>
        public async Task UpdateHostingRoomStatusAsync(RoomStatus roomStatus)
        {
            try
            {
                await Task.Run(() => _client.UpdateHostingRoomStatusAsync(roomStatus));
            }
            catch (Exception e)
            {
                Debug.LogException(e);
                throw;
            }
        }

        /// <summary>
        /// Remove hosting room from the server.
        /// </summary>
        /// <exception cref="ClientErrorException"></exception>
        /// <returns></returns>
        public async Task RemoveHostingRoomAsync()
        {
            try
            {
                await Task.Run(() => _client.RemoveHostingRoomAsync());
            }
            catch (Exception e)
            {
                Debug.LogException(e);
                throw;
            }
        }

        public void Dispose()
        {
            _client.Dispose();
        }

        [SerializeField, Tooltip("IP Address of Match Making Server")]
        private string _serverAddress = "127.0.0.1";

        [SerializeField, Tooltip("Port of Match Making Server")]
        private ushort _serverPort = 57000;

        private MatchMakerClient _client;

        private void Awake()
        {
            _client = new MatchMakerClient();
        }

        private void OnDestroy()
        {
            _client.Dispose();
        }
    }
}