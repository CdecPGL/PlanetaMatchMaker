using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Net;
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

        public ushort gameDefaultPort => _gameDefaultPort;

        [Serializable]
        public struct PortRange
        {
            public ushort startPort => _startPort;
            public ushort endPort => _endPort;

            [SerializeField] private ushort _startPort;
            [SerializeField] private ushort _endPort;
        }

        public IReadOnlyList<PortRange> gamePortCandidates => _gamePortCandidates.AsReadOnly();

        public byte roomGroupIndex { get; set; }

        public IReadOnlyList<IRoomGroupInfo> roomGroupInfoList => _roomGroupInfoList.AsReadOnly();

        public IHostingRoomInfo hostingRoomInfo { get; private set; }

        public PlayerFullName playerFullName => _client.PlayerFullName;

        public struct ConnectCallbackArgs
        {
            public ConnectCallbackArgs(PlayerFullName playerFullName)
            {
                this.playerFullName = playerFullName;
            }

            public readonly PlayerFullName playerFullName;
        }

        /// <summary>
        /// Connect to the matching server
        /// </summary>
        /// <param name="playerName"></param>
        /// <param name="callback"></param>
        public void Connect(string playerName, Action<ErrorInfo, ConnectCallbackArgs> callback = null)
        {
            if (status != Status.Disconnected)
            {
                Debug.LogError("Client is already started.");
                return;
            }

            RunTaskWithErrorHandling(async () =>
            {
                status = Status.Connecting;
                var returnedPlayerFullName = await ConnectAsync(playerName);
                status = Status.SearchingRoom;
                await GetRoomGroupListAsync();
                return new ConnectCallbackArgs(returnedPlayerFullName);
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

            if (_client.Connected)
            {
                _client.Close();
            }

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
                    var (totalRoomCount, _, startIndex, roomInfoList) = await GetRoomListAsync(resultStartIndex,
                        resultCount, sortKind, targetFlags, searchName);
                    return new RequestRoomListCallbackArgs(totalRoomCount, startIndex,
                        roomInfoList);
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
        /// <param name="maxPlayerCount"></param>
        /// <param name="isPublic"></param>
        /// <param name="password"></param>
        /// <param name="callback"></param>
        /// <exception cref="ClientErrorException"></exception>
        /// <returns></returns>
        public void HostRoom(byte maxPlayerCount, bool isPublic = true, string password = "",
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
                await CreateRoomAsync(maxPlayerCount, isPublic, password);
                status = Status.HostingRoom;
                return new HostRoomCallbackArgs(hostingRoomInfo);
            }, () => status = Status.SearchingRoom, callback);
        }

        public struct HostRoomWithCreatingPortMappingCallbackArgs
        {
            public HostRoomWithCreatingPortMappingCallbackArgs(IHostingRoomInfo hostRoomInfo, bool isDefaultPortUsed,
                ushort privatePort, ushort publicPort)
            {
                this.hostRoomInfo = hostRoomInfo;
                this.isDefaultPortUsed = isDefaultPortUsed;
                this.privatePort = privatePort;
                this.publicPort = publicPort;
            }

            public readonly IHostingRoomInfo hostRoomInfo;
            public readonly bool isDefaultPortUsed;
            public readonly ushort privatePort;
            public readonly ushort publicPort;
        }

        /// <summary>
        /// Create and host new room to the server with trying to create port mapping.
        /// </summary>
        /// <param name="maxPlayerCount"></param>
        /// <param name="isPublic"></param>
        /// <param name="password"></param>
        /// <param name="callback"></param>
        /// <exception cref="ClientErrorException"></exception>
        /// <returns></returns>
        public void HostRoomWithCreatingPortMapping(byte maxPlayerCount, bool isPublic = true, string password = "",
            Action<ErrorInfo, HostRoomWithCreatingPortMappingCallbackArgs> callback = null)
        {
            if (status != Status.SearchingRoom)
            {
                Debug.LogError("The operation is valid when searching room.");
                return;
            }

            RunTaskWithErrorHandling(async () =>
            {
                status = Status.StartingHostingRoom;
                var (isDefaultPortUsed, privatePort, publicPort) =
                    await CreateRoomWithCreatingPortMappingAsync(maxPlayerCount, isPublic, password);
                status = Status.HostingRoom;
                return new HostRoomWithCreatingPortMappingCallbackArgs(hostingRoomInfo, isDefaultPortUsed, privatePort,
                    publicPort);
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
            public JoinRoomCallbackArgs(IPEndPoint roomGameHostEndPoint)
            {
                this.roomGameHostEndPoint = roomGameHostEndPoint;
            }

            public readonly IPEndPoint roomGameHostEndPoint;
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
                hostPlayerFullName = roomResult.HostPlayerFullName;
                settingFlags = roomResult.SettingFlags;
                maxPlayerCount = roomResult.MaxPlayerCount;
                currentPlayerCount = roomResult.CurrentPlayerCount;
                createDatetime = roomResult.CreateDatetime;
            }

            public byte roomGroupIndex { get; }
            public uint roomId { get; }
            public PlayerFullName hostPlayerFullName { get; }
            public RoomSettingFlag settingFlags { get; }
            public byte maxPlayerCount { get; }
            public byte currentPlayerCount { get; }
            public Datetime createDatetime { get; }
        }

        private sealed class HostingRoomInfo : IHostingRoomInfo
        {
            public HostingRoomInfo(byte roomGroupIndex, bool isPublic, uint roomId, byte maxPlayerCount)
            {
                this.roomGroupIndex = roomGroupIndex;
                this.roomId = roomId;
                this.maxPlayerCount = maxPlayerCount;
                this.isPublic = isPublic;
            }

            public byte roomGroupIndex { get; }
            public uint roomId { get; }
            public byte maxPlayerCount { get; }
            public bool isPublic { get; }
        }

        [SerializeField, Tooltip("IP Address of Match Making Server")]
        private string _serverAddress = "127.0.0.1";

        [SerializeField, Tooltip("Port of Match Making Server")]
        private ushort _serverPort = 57000;

        [SerializeField, Tooltip("Port of Game Host")]
        private ushort _gameDefaultPort = 52000;

        [SerializeField, Tooltip("Candidates of ports used for game port")]
        private List<PortRange> _gamePortCandidates;

        [SerializeField, Tooltip("Timeout seconds to discover NAT device")]
        private float _natDiscoverTimeOutSeconds = 5;

        [SerializeField, Tooltip("Timeout seconds to send and receive data between match making server")]
        private float _serverCommunicationTimeOutSeconds = 10;

        [SerializeField, Tooltip("Enable debug log")]
        private bool _enableDebugLog;

        private MatchMakerClient _client;

        private List<RoomGroupInfo> _roomGroupInfoList = new List<RoomGroupInfo>();
        private Task _task;

        private void Awake()
        {
            _client = new MatchMakerClient((int)(_serverCommunicationTimeOutSeconds * 1000), new UnityLogger());
            _client.Logger.Enabled = _enableDebugLog;
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
                            if (!ce.ClientErrorCode.IsContinuable())
                            {
                                Disconnect();
                            }

                            break;
                        default:
                            callback?.Invoke(new ErrorInfo(e), defaultResult);
                            break;
                    }
                }
            }

            _task = Task.Run(Wrapper);
        }

        private async Task<PlayerFullName> ConnectAsync(string playerName)
        {
            return await _client.ConnectAsync(_serverAddress, _serverPort, playerName);
        }

        private async Task GetRoomGroupListAsync()
        {
            _roomGroupInfoList = (await _client.GetRoomGroupListAsync()).Select(result => new RoomGroupInfo(result))
                .ToList();
        }

        private async Task CreateRoomAsync(byte maxPlayerCount, bool isPublic = true,
            string password = "")
        {
            await _client.CreateRoomAsync(roomGroupIndex, maxPlayerCount, _gameDefaultPort, isPublic,
                password);
            hostingRoomInfo = new HostingRoomInfo(_client.HostingRoomGroupIndex, isPublic, _client.HostingRoomId,
                maxPlayerCount);
        }

        private async Task<(bool isDefaultPortUsed, ushort privatePort, ushort publicPort)>
            CreateRoomWithCreatingPortMappingAsync(byte maxPlayerCount, bool isPublic = true, string password = "")
        {
            var result = await _client.CreateRoomWithCreatingPortMappingAsync(roomGroupIndex, maxPlayerCount,
                TransportProtocol.Tcp, GenerateGamePortCandidateList(), _gameDefaultPort,
                (int)(_natDiscoverTimeOutSeconds * 1000), isPublic, password);
            hostingRoomInfo = new HostingRoomInfo(_client.HostingRoomGroupIndex, isPublic, _client.HostingRoomId,
                maxPlayerCount);
            return result.IsDefaultPortUsed
                ? (true, _gameDefaultPort, _gameDefaultPort)
                : (false, result.UsedPrivatePortFromCandidates, result.UsedPublicPortFromCandidates);
        }

        private async Task<IPEndPoint> JoinRoomAsync(uint roomId, string password = "")
        {
            var roomGameHostEndPoint = await _client.JoinRoomAsync(roomGroupIndex, roomId, password);
            return roomGameHostEndPoint;
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

        private async Task<(byte totalRoomCount, byte matchedRoomCount, byte startIndex, IReadOnlyList<IRoomInfo>
                roomInfoList)>
            GetRoomListAsync(byte resultStartIndex, byte resultCount, RoomDataSortKind sortKind,
                RoomSearchTargetFlag targetFlags, string searchName)
        {
            var hostRoomGroupIndex = roomGroupIndex;
            var (totalRoomCount, matchedRoomCount, roomInfoList) = await _client.GetRoomListAsync(hostRoomGroupIndex,
                resultStartIndex, resultCount, sortKind, targetFlags, searchName);
            return (totalRoomCount, resultStartIndex, matchedRoomCount,
                roomInfoList.Select(result => new RoomInfo(hostRoomGroupIndex, result)).ToList().AsReadOnly());
        }

        private List<ushort> GenerateGamePortCandidateList()
        {
            var list = new List<ushort>();
            foreach (var gamePortCandidate in _gamePortCandidates)
            {
                list.AddRange(Enumerable
                    .Range(gamePortCandidate.startPort, gamePortCandidate.endPort - gamePortCandidate.startPort + 1)
                    .Select(v => (ushort)v));
            }

            return list;
        }

        private void Reset()
        {
            status = Status.Disconnected;
            hostingRoomInfo = null;
            _roomGroupInfoList.Clear();
        }
    }
}