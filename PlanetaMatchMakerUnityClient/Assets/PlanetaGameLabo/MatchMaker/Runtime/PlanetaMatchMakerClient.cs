using System;
using System.Collections.Generic;
using System.Linq;
using System.Net.Sockets;
using System.Threading.Tasks;
using UnityEngine;

namespace PlanetaGameLabo.MatchMaker
{
    [AddComponentMenu("PlanetaGameLabo/MatchMaker/PlanetaMatchMakerClient"), DisallowMultipleComponent]
    public sealed class PlanetaMatchMakerClient : MonoBehaviour, IDisposable
    {
        public enum Status
        {
            Disconnected, Connecting, SearchingRoom, StartingHostingRoom, HostingRoom, FinishingHostingRoom,
            StartingJoiningRoom
        }

        public struct ErrorInfo
        {
            public ErrorInfo(ClientErrorCode errorCode)
            {
                this.errorCode = errorCode;
                message = "";
            }

            public ErrorInfo(Exception e)
            {
                switch (e)
                {
                    case ClientErrorException ce:
                        errorCode = ce.ClientErrorCode;
                        break;
                    case SocketException _:
                        errorCode = ClientErrorCode.ConnectionClosed;
                        break;
                    default:
                        errorCode = ClientErrorCode.UnknownError;
                        break;
                }

                message = e.Message;
            }

            public readonly ClientErrorCode errorCode;
            public readonly string message;

            public static bool operator true(ErrorInfo e) { return e.errorCode == ClientErrorCode.Ok; }
            public static bool operator false(ErrorInfo e) { return e.errorCode != ClientErrorCode.Ok; }
            public static implicit operator bool(ErrorInfo e) { return e.errorCode == ClientErrorCode.Ok; }
        }

        /// <summary>
        /// A status of client.
        /// </summary>
        public Status status { get; private set; } = Status.Disconnected;

        /// <summary>
        /// true if connected to the server.
        /// </summary>
        public bool connected => _client?.Connected ?? false;

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

        public byte roomGroupIndex { get; set; }

        public IReadOnlyList<IRoomGroupInfo> roomGroupInfoList => _roomGroupInfoList.AsReadOnly();

        public IHostingRoomInfo hostingRoomInfo { get; private set; }

        /// <summary>
        /// Connect to the matching server
        /// </summary>
        /// <param name="callback"></param>
        public void Connect(Action<ErrorInfo> callback = null)
        {
            if (status != Status.Disconnected)
            {
                Debug.LogError("Client is already started.");
                return;
            }

            RunTaskWithErrorHandling(async () =>
            {
                status = Status.Connecting;
                await ConnectAsync();
                status = Status.SearchingRoom;
                await GetRoomGroupListAsync();
            }, () =>
            {
                if (status == Status.Connecting)
                {
                    status = Status.Disconnected;
                }
            }, callback);
        }

        /// <summary>
        /// Close the connection.
        /// </summary>
        public void Disconnect()
        {
            if (status == Status.Disconnected)
            {
                Debug.LogError("Client is already stopped.");
                return;
            }

            _client.Close();
            Reset();
        }

        public struct RequestRoomListCallbackArgs
        {
            public RequestRoomListCallbackArgs(byte totalRoomCount, byte startIndex,
                IReadOnlyList<IRoomInfo> roomInfoList)
            {
                this.totalRoomCount = totalRoomCount;
                this.startIndex = startIndex;
                this.roomInfoList = roomInfoList;
            }

            public readonly byte totalRoomCount;
            public readonly byte startIndex;
            public readonly IReadOnlyList<IRoomInfo> roomInfoList;
        }

        /// <summary>
        /// Request room list to the server.
        /// </summary>
        /// <param name="resultStartIndex"></param>
        /// <param name="resultCount"></param>
        /// <param name="sortKind"></param>
        /// <param name="targetFlags"></param>
        /// <param name="searchName"></param>
        /// <param name="callback"></param>
        public void RequestRoomList(byte resultStartIndex, byte resultCount, RoomDataSortKind sortKind,
            RoomSearchTargetFlag targetFlags, string searchName = "",
            Action<ErrorInfo, RequestRoomListCallbackArgs> callback = null)
        {
            if (status != Status.SearchingRoom)
            {
                Debug.LogError("The operation is valid when searching room.");
                return;
            }

            if (_roomGroupInfoList.Count == 0)
            {
                Debug.LogError("There are no room group.");
                return;
            }

            RunTaskWithErrorHandling(async () =>
                {
                    var result = await GetRoomListAsync(resultStartIndex, resultCount, sortKind, targetFlags,
                        searchName);
                    return new RequestRoomListCallbackArgs(result.totalRoomCount, result.startIndex,
                        result.roomInfoList);
                },
                () => { },
                callback);
        }

        public struct HostRoomCallbackArgs
        {
            public HostRoomCallbackArgs(IHostingRoomInfo hostRoomInfo)
            {
                this.hostRoomInfo = hostRoomInfo;
            }

            public readonly IHostingRoomInfo hostRoomInfo;
        }

        /// <summary>
        /// Create and host new room to the server.
        /// </summary>
        /// <param name="roomName"></param>
        /// <param name="maxPlayerCount"></param>
        /// <param name="isPublic"></param>
        /// <param name="password"></param>
        /// <param name="callback"></param>
        /// <exception cref="ClientErrorException"></exception>
        /// <returns></returns>
        public void HostRoom(string roomName, byte maxPlayerCount, bool isPublic = true, string password = "",
            Action<ErrorInfo, HostRoomCallbackArgs> callback = null)
        {
            if (status != Status.SearchingRoom)
            {
                Debug.LogError("The operation is valid when searching room.");
                return;
            }

            RunTaskWithErrorHandling(async () =>
            {
                status = Status.StartingHostingRoom;
                await CreateRoomAsync(roomName, maxPlayerCount, isPublic, password);
                status = Status.HostingRoom;
                return new HostRoomCallbackArgs(hostingRoomInfo);
            }, () => status = Status.SearchingRoom, callback);
        }

        public void CloseHostingRoom(Action<ErrorInfo> callback = null)
        {
            if (status != Status.HostingRoom)
            {
                Debug.LogError("Not hosting room.");
                return;
            }

            RunTaskWithErrorHandling(async () => await UpdateHostingRoomStatusAsync(RoomStatus.Close), () => { },
                callback);
        }

        public void CloseHostingRoom(byte currentPlayerCount, Action<ErrorInfo> callback = null)
        {
            if (status != Status.HostingRoom)
            {
                Debug.LogError("Not hosting room.");
                return;
            }

            RunTaskWithErrorHandling(
                async () => await UpdateHostingRoomStatusAsync(RoomStatus.Close, true, currentPlayerCount), () => { },
                callback);
        }

        public void OpenHostingRoom(Action<ErrorInfo> callback = null)
        {
            if (status != Status.HostingRoom)
            {
                Debug.LogError("Not hosting room.");
                return;
            }

            RunTaskWithErrorHandling(async () => await UpdateHostingRoomStatusAsync(RoomStatus.Open), () => { },
                callback);
        }

        public void OpenHostingRoom(byte currentPlayerCount, Action<ErrorInfo> callback = null)
        {
            if (status != Status.HostingRoom)
            {
                Debug.LogError("Not hosting room.");
                return;
            }

            RunTaskWithErrorHandling(
                async () => await UpdateHostingRoomStatusAsync(RoomStatus.Open, true, currentPlayerCount), () => { },
                callback);
        }

        public void ChangeOpenOrCloseHostingRoom(bool isOpen, Action<ErrorInfo> callback = null)
        {
            if (status != Status.HostingRoom)
            {
                Debug.LogError("Not hosting room.");
                return;
            }

            RunTaskWithErrorHandling(
                async () => await UpdateHostingRoomStatusAsync(isOpen ? RoomStatus.Open : RoomStatus.Close), () => { },
                callback);
        }

        public void ChangeOpenOrCloseHostingRoom(bool isOpen, byte currentPlayerCount,
            Action<ErrorInfo> callback = null)
        {
            if (status != Status.HostingRoom)
            {
                Debug.LogError("Not hosting room.");
                return;
            }

            RunTaskWithErrorHandling(
                async () => await UpdateHostingRoomStatusAsync(isOpen ? RoomStatus.Open : RoomStatus.Close, true,
                    currentPlayerCount), () => { }, callback);
        }

        public void RemoveHostingRoom(Action<ErrorInfo> callback = null)
        {
            if (status != Status.HostingRoom)
            {
                Debug.LogError("Not hosting room.");
                return;
            }

            RunTaskWithErrorHandling(async () =>
            {
                status = Status.FinishingHostingRoom;
                await RemoveHostingRoomAsync();
                status = Status.SearchingRoom;
                hostingRoomInfo = null;
            }, () => status = Status.HostingRoom, callback);
        }

        public struct JoinRoomCallbackArgs
        {
            public JoinRoomCallbackArgs(ClientAddress roomHostClientAddress)
            {
                this.roomHostClientAddress = roomHostClientAddress;
            }

            public readonly ClientAddress roomHostClientAddress;
        }

        /// <summary>
        /// Join to a room on the server.
        /// </summary>
        /// <param name="roomId"></param>
        /// <param name="password"></param>
        /// <param name="callback"></param>
        /// <exception cref="ClientErrorException"></exception>
        /// <returns></returns>
        public void JoinRoom(uint roomId, string password = "",
            Action<ErrorInfo, JoinRoomCallbackArgs> callback = null)
        {
            if (status != Status.SearchingRoom)
            {
                Debug.LogError("The operation is valid when searching room.");
                return;
            }

            RunTaskWithErrorHandling(async () =>
            {
                status = Status.StartingJoiningRoom;
                var roomHostClientAddress = await JoinRoomAsync(roomId, password);
                Disconnect();
                return new JoinRoomCallbackArgs(roomHostClientAddress);
            }, () => status = Status.SearchingRoom, callback);
        }

        public void Dispose()
        {
            _client.Dispose();
            Reset();
        }

        private sealed class RoomGroupInfo : IRoomGroupInfo
        {
            public RoomGroupInfo(RoomGroupResult roomGroupResult)
            {
                name = roomGroupResult.Name;
            }

            public string name { get; }
        }

        private sealed class RoomInfo : IRoomInfo
        {
            public RoomInfo(byte roomGroupIndex, RoomResult roomResult)
            {
                this.roomGroupIndex = roomGroupIndex;
                roomId = roomResult.RoomId;
                name = roomResult.Name;
                settingFlags = roomResult.SettingFlags;
                maxPlayerCount = roomResult.MaxPlayerCount;
                currentPlayerCount = roomResult.CurrentPlayerCount;
                createDatetime = roomResult.CreateDatetime;
            }

            public byte roomGroupIndex { get; }
            public uint roomId { get; }
            public string name { get; }
            public RoomSettingFlag settingFlags { get; }
            public byte maxPlayerCount { get; }
            public byte currentPlayerCount { get; }
            public Datetime createDatetime { get; }
        }

        private sealed class HostingRoomInfo : IHostingRoomInfo
        {
            public HostingRoomInfo(byte roomGroupIndex, bool isPublic, uint roomId, string name, byte maxPlayerCount)
            {
                this.roomGroupIndex = roomGroupIndex;
                this.roomId = roomId;
                this.name = name;
                this.maxPlayerCount = maxPlayerCount;
                this.isPublic = isPublic;
            }

            public byte roomGroupIndex { get; }
            public uint roomId { get; }
            public string name { get; }
            public byte maxPlayerCount { get; }
            public bool isPublic { get; }
        }

        [SerializeField, Tooltip("IP Address of Match Making Server")]
        private string _serverAddress = "127.0.0.1";

        [SerializeField, Tooltip("Port of Match Making Server")]
        private ushort _serverPort = 57000;

        private MatchMakerClient _client;

        private List<RoomGroupInfo> _roomGroupInfoList = new List<RoomGroupInfo>();
        private Task _task;

        private void Awake()
        {
            _client = new MatchMakerClient();
        }

        private void OnDestroy()
        {
            _client.Dispose();
        }

        private void RunTaskWithErrorHandling(Func<Task> task, Action errorHandler,
            Action<ErrorInfo> callback)
        {
            async Task Wrapper()
            {
                try
                {
                    await task();
                    callback?.Invoke(new ErrorInfo(ClientErrorCode.Ok));
                }
                catch (Exception e)
                {
                    Debug.LogException(e);
                    errorHandler?.Invoke();
                    switch (e)
                    {
                        case ClientErrorException ce:
                            callback?.Invoke(new ErrorInfo(e));
                            if (ce.ClientErrorCode == ClientErrorCode.ConnectionClosed)
                            {
                                Disconnect();
                            }

                            break;
                        default:
                            callback?.Invoke(new ErrorInfo(e));
                            break;
                    }
                }
            }

            _task = Task.Run(Wrapper);
        }

        private void RunTaskWithErrorHandling<T>(Func<Task<T>> task, Action errorHandler,
            Action<ErrorInfo, T> callback, T defaultResult = default)
        {
            async Task Wrapper()
            {
                try
                {
                    var result = await task();
                    callback?.Invoke(new ErrorInfo(ClientErrorCode.Ok), result);
                }
                catch (Exception e)
                {
                    Debug.LogException(e);
                    errorHandler?.Invoke();
                    switch (e)
                    {
                        case ClientErrorException ce:
                            callback?.Invoke(new ErrorInfo(e), defaultResult);
                            if (ce.ClientErrorCode == ClientErrorCode.ConnectionClosed)
                            {
                                Disconnect();
                            }

                            break;
                        case ClientInternalErrorException _:
                            callback?.Invoke(new ErrorInfo(e), defaultResult);
                            Disconnect();
                            break;
                        default:
                            callback?.Invoke(new ErrorInfo(e), defaultResult);
                            break;
                    }
                }
            }

            _task = Task.Run(Wrapper);
        }

        private async Task ConnectAsync()
        {
            await _client.ConnectAsync(_serverAddress, _serverPort);
        }

        private async Task GetRoomGroupListAsync()
        {
            _roomGroupInfoList = (await _client.GetRoomGroupListAsync()).Select(result => new RoomGroupInfo(result))
                .ToList();
        }

        private async Task CreateRoomAsync(string roomName, byte maxPlayerCount, bool isPublic = true,
            string password = "")
        {
            await _client.CreateRoomAsync(roomGroupIndex, roomName, maxPlayerCount, isPublic, password);
            hostingRoomInfo = new HostingRoomInfo(_client.HostingRoomGroupIndex, isPublic, _client.HostingRoomId,
                roomName, maxPlayerCount);
        }

        private async Task<ClientAddress> JoinRoomAsync(uint roomId, string password = "")
        {
            var roomHostClientAddress = await _client.JoinRoomAsync(roomGroupIndex, roomId, password);
            return roomHostClientAddress;
        }

        private async Task UpdateHostingRoomStatusAsync(RoomStatus roomStatus, bool updateCurrentPlayerCount = false,
            byte currentPlayerCount = 0)
        {
            await _client.UpdateHostingRoomStatusAsync(roomStatus, updateCurrentPlayerCount, currentPlayerCount);
        }

        private async Task RemoveHostingRoomAsync()
        {
            await _client.RemoveHostingRoomAsync();
        }

        private async Task<(byte totalRoomCount, byte startIndex, IReadOnlyList<IRoomInfo> roomInfoList)>
            GetRoomListAsync(byte resultStartIndex, byte resultCount, RoomDataSortKind sortKind,
                RoomSearchTargetFlag targetFlags, string searchName)
        {
            var hostRoomGroupIndex = roomGroupIndex;
            var (totalRoomCount, roomInfoList) = await _client.GetRoomListAsync(hostRoomGroupIndex, resultStartIndex,
                resultCount, sortKind, targetFlags, searchName);
            return (totalRoomCount, resultStartIndex,
                roomInfoList.Select(result => new RoomInfo(hostRoomGroupIndex, result)).ToList().AsReadOnly());
        }

        private void Reset()
        {
            status = Status.Disconnected;
            hostingRoomInfo = null;
            _roomGroupInfoList.Clear();
        }
    }
}