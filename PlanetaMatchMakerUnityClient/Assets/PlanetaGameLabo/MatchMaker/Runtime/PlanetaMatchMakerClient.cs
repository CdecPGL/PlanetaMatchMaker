using System;
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
        #region PublicTypes

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

        #endregion

        #region Properties

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

        public IReadOnlyList<RoomGroupInfo> roomGroupInfoList => _roomGroupInfoList.AsReadOnly();

        public HostingRoomInfo hostingRoomInfo { get; private set; }

        public PlayerFullName playerFullName => _client.PlayerFullName;

        #endregion

        #region Operations

        /// <summary>
        /// Connect to the matching server
        /// </summary>
        /// <param name="playerName"></param>
        /// <param name="callback"></param>
        public void Connect(string playerName, Action<ErrorInfo, ConnectCallbackArgs> callback = null)
        {
            RunTask(async () => await ConnectAsync(playerName), callback);
        }

        /// <summary>
        /// Connect to the matching server
        /// </summary>
        /// <param name="playerName"></param>
        public async Task<(ErrorInfo errorInfo, ConnectCallbackArgs result)> ConnectAsync(string playerName)
        {
            if (status != Status.Disconnected)
            {
                Debug.LogError("Client is already started.");
                return (new ErrorInfo(ClientErrorCode.InvalidOperation), default);
            }

            return await RunTaskWithErrorHandlingAsync(async () =>
            {
                status = Status.Connecting;
                var returnedPlayerFullName = await ConnectImplAsync(playerName);
                status = Status.SearchingRoom;
                await GetRoomGroupListImplAsync();
                return new ConnectCallbackArgs(returnedPlayerFullName);
            }, () =>
            {
                if (status == Status.Connecting)
                {
                    status = Status.Disconnected;
                }
            });
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
            RunTask(
                async () => await RequestRoomListAsync(resultStartIndex, resultCount, sortKind, targetFlags,
                    searchName), callback);
        }

        /// <summary>
        /// Request room list to the server.
        /// </summary>
        /// <param name="resultStartIndex"></param>
        /// <param name="resultCount"></param>
        /// <param name="sortKind"></param>
        /// <param name="targetFlags"></param>
        /// <param name="searchName"></param>
        public async Task<(ErrorInfo errorInfo, RequestRoomListCallbackArgs result)> RequestRoomListAsync(
            byte resultStartIndex, byte resultCount, RoomDataSortKind sortKind,
            RoomSearchTargetFlag targetFlags, string searchName = "")
        {
            if (status != Status.SearchingRoom)
            {
                Debug.LogError("The operation is valid when searching room.");
                return (new ErrorInfo(ClientErrorCode.InvalidOperation), default);
            }

            if (_roomGroupInfoList.Count == 0)
            {
                Debug.LogError("There are no room group.");
                return (new ErrorInfo(ClientErrorCode.InvalidOperation), default);
            }

            return await RunTaskWithErrorHandlingAsync(async () =>
                {
                    var (totalRoomCount, _, startIndex, roomInfoList) = await GetRoomListAsync(resultStartIndex,
                        resultCount, sortKind, targetFlags, searchName);
                    return new RequestRoomListCallbackArgs(totalRoomCount, startIndex,
                        roomInfoList);
                },
                () => { });
        }

        /// <summary>
        /// Create and host new room to the server.
        /// </summary>
        /// <param name="maxPlayerCount"></param>
        /// <param name="isPublic"></param>
        /// <param name="password"></param>
        /// <param name="callback"></param>
        /// <returns></returns>
        public void HostRoom(byte maxPlayerCount, bool isPublic = true, string password = "",
            Action<ErrorInfo, HostRoomCallbackArgs> callback = null)
        {
            RunTask(async () => await HostRoomAsync(maxPlayerCount, isPublic, password), callback);
        }

        /// <summary>
        /// Create and host new room to the server.
        /// </summary>
        /// <param name="maxPlayerCount"></param>
        /// <param name="isPublic"></param>
        /// <param name="password"></param>
        /// <returns></returns>
        public async Task<(ErrorInfo errorInfo, HostRoomCallbackArgs result)> HostRoomAsync(byte maxPlayerCount,
            bool isPublic = true, string password = "")
        {
            if (status != Status.SearchingRoom)
            {
                Debug.LogError("The operation is valid when searching room.");
                return (new ErrorInfo(ClientErrorCode.InvalidOperation), default);
            }

            return await RunTaskWithErrorHandlingAsync(async () =>
            {
                status = Status.StartingHostingRoom;
                await CreateRoomImplAsync(maxPlayerCount, isPublic, password);
                status = Status.HostingRoom;
                return new HostRoomCallbackArgs(hostingRoomInfo);
            }, () => status = Status.SearchingRoom);
        }

        /// <summary>
        /// Create and host new room to the server with trying to create port mapping.
        /// </summary>
        /// <param name="maxPlayerCount"></param>
        /// <param name="isPublic"></param>
        /// <param name="password"></param>
        /// <param name="callback"></param>
        /// <returns></returns>
        public void HostRoomWithCreatingPortMapping(byte maxPlayerCount, bool isPublic = true, string password = "",
            Action<ErrorInfo, HostRoomWithCreatingPortMappingCallbackArgs> callback = null)
        {
            RunTask(async () => await HostRoomWithCreatingPortMappingAsync(maxPlayerCount, isPublic, password),
                callback);
        }

        /// <summary>
        /// Create and host new room to the server with trying to create port mapping.
        /// </summary>
        /// <param name="maxPlayerCount"></param>
        /// <param name="isPublic"></param>
        /// <param name="password"></param>
        /// <returns></returns>
        public async Task<(ErrorInfo errorInfo, HostRoomWithCreatingPortMappingCallbackArgs result)>
            HostRoomWithCreatingPortMappingAsync(byte maxPlayerCount, bool isPublic = true, string password = "")
        {
            if (status != Status.SearchingRoom)
            {
                Debug.LogError("The operation is valid when searching room.");
                return (new ErrorInfo(ClientErrorCode.InvalidOperation), default);
            }

            return await RunTaskWithErrorHandlingAsync(async () =>
            {
                status = Status.StartingHostingRoom;
                var (isDefaultPortUsed, privatePort, publicPort) =
                    await CreateRoomWithCreatingPortMappingImplAsync(maxPlayerCount, isPublic, password);
                status = Status.HostingRoom;
                return new HostRoomWithCreatingPortMappingCallbackArgs(hostingRoomInfo, isDefaultPortUsed, privatePort,
                    publicPort);
            }, () => status = Status.SearchingRoom);
        }

        /// <summary>
        /// Close hosting room.
        /// </summary>
        /// <param name="callback"></param>
        public void CloseHostingRoom(Action<ErrorInfo> callback = null)
        {
            RunTask(async () => await CloseHostingRoomAsync(), callback);
        }

        /// <summary>
        /// Close hosting room.
        /// </summary>
        /// <returns></returns>
        public async Task<ErrorInfo> CloseHostingRoomAsync()
        {
            if (status != Status.HostingRoom)
            {
                Debug.LogError("Not hosting room.");
                return new ErrorInfo(ClientErrorCode.InvalidOperation);
            }

            return await RunTaskWithErrorHandlingAsync(
                async () => await UpdateHostingRoomStatusImplAsync(RoomStatus.Close), () => { });
        }

        /// <summary>
        /// Close hosting room with updating current player count.
        /// </summary>
        /// <param name="currentPlayerCount"></param>
        /// <param name="callback"></param>
        public void CloseHostingRoom(byte currentPlayerCount, Action<ErrorInfo> callback = null)
        {
            if (status != Status.HostingRoom)
            {
                Debug.LogError("Not hosting room.");
                return;
            }

            RunTask(async () => await CloseHostingRoomAsync(currentPlayerCount), callback);
        }


        /// <summary>
        /// Close hosting room with updating current player count.
        /// </summary>
        /// <param name="currentPlayerCount"></param>
        public async Task<ErrorInfo> CloseHostingRoomAsync(byte currentPlayerCount)
        {
            if (status != Status.HostingRoom)
            {
                Debug.LogError("Not hosting room.");
                return new ErrorInfo(ClientErrorCode.InvalidOperation);
            }

            return await RunTaskWithErrorHandlingAsync(
                async () => await UpdateHostingRoomStatusImplAsync(RoomStatus.Close, true, currentPlayerCount),
                () => { });
        }

        /// <summary>
        /// Open hosting room.
        /// </summary>
        /// <param name="callback"></param>
        public void OpenHostingRoom(Action<ErrorInfo> callback = null)
        {
            RunTask(async () => await OpenHostingRoomAsync(), callback);
        }

        /// <summary>
        /// Open hosting room.
        /// </summary>
        public async Task<ErrorInfo> OpenHostingRoomAsync()
        {
            if (status != Status.HostingRoom)
            {
                Debug.LogError("Not hosting room.");
                return new ErrorInfo(ClientErrorCode.InvalidOperation);
            }

            return await RunTaskWithErrorHandlingAsync(
                async () => await UpdateHostingRoomStatusImplAsync(RoomStatus.Open), () => { });
        }

        /// <summary>
        /// Open hosting room with updating current player count.
        /// </summary>
        /// <param name="currentPlayerCount"></param>
        /// <param name="callback"></param>
        public void OpenHostingRoom(byte currentPlayerCount, Action<ErrorInfo> callback = null)
        {
            RunTask(async () => await OpenHostingRoomAsync(currentPlayerCount), callback);
        }

        /// <summary>
        /// Open hosting room with updating current player count.
        /// </summary>
        /// <param name="currentPlayerCount"></param>
        public async Task<ErrorInfo> OpenHostingRoomAsync(byte currentPlayerCount)
        {
            if (status != Status.HostingRoom)
            {
                Debug.LogError("Not hosting room.");
                return new ErrorInfo(ClientErrorCode.InvalidOperation);
            }

            return await RunTaskWithErrorHandlingAsync(
                async () => await UpdateHostingRoomStatusImplAsync(RoomStatus.Open, true, currentPlayerCount),
                () => { });
        }

        /// <summary>
        /// Change open status of hosting room.
        /// </summary>
        /// <param name="isOpen"></param>
        /// <param name="callback"></param>
        public void ChangeOpenOrCloseHostingRoom(bool isOpen, Action<ErrorInfo> callback = null)
        {
            RunTask(async () => await ChangeOpenOrCloseHostingRoomAsync(isOpen), callback);
        }

        /// <summary>
        /// Change open status of hosting room.
        /// </summary>
        /// <param name="isOpen"></param>
        public async Task<ErrorInfo> ChangeOpenOrCloseHostingRoomAsync(bool isOpen)
        {
            if (status != Status.HostingRoom)
            {
                Debug.LogError("Not hosting room.");
                return new ErrorInfo(ClientErrorCode.InvalidOperation);
            }

            return await RunTaskWithErrorHandlingAsync(
                async () => await UpdateHostingRoomStatusImplAsync(isOpen ? RoomStatus.Open : RoomStatus.Close),
                () => { });
        }

        /// <summary>
        /// Change open status of hosting room with updating current player count.
        /// </summary>
        /// <param name="isOpen"></param>
        /// <param name="currentPlayerCount"></param>
        /// <param name="callback"></param>
        public void ChangeOpenOrCloseHostingRoom(bool isOpen, byte currentPlayerCount,
            Action<ErrorInfo> callback = null)
        {
            RunTask(
                async () => await ChangeOpenOrCloseHostingRoomAsync(isOpen, currentPlayerCount), callback);
        }

        /// <summary>
        /// Change open status of hosting room with updating current player count.
        /// </summary>
        /// <param name="isOpen"></param>
        /// <param name="currentPlayerCount"></param>
        public async Task<ErrorInfo> ChangeOpenOrCloseHostingRoomAsync(bool isOpen, byte currentPlayerCount)
        {
            if (status != Status.HostingRoom)
            {
                Debug.LogError("Not hosting room.");
                return new ErrorInfo(ClientErrorCode.InvalidOperation);
            }

            return await RunTaskWithErrorHandlingAsync(
                async () => await UpdateHostingRoomStatusImplAsync(isOpen ? RoomStatus.Open : RoomStatus.Close, true,
                    currentPlayerCount), () => { });
        }

        /// <summary>
        /// Remove hosting room.
        /// </summary>
        /// <param name="callback"></param>
        public void RemoveHostingRoom(Action<ErrorInfo> callback = null)
        {
            RunTask(async () => await RemoveHostingRoomAsync(), callback);
        }

        /// <summary>
        /// Remove hosting room.
        /// </summary>
        public async Task<ErrorInfo> RemoveHostingRoomAsync()
        {
            if (status != Status.HostingRoom)
            {
                Debug.LogError("Not hosting room.");
                return new ErrorInfo(ClientErrorCode.InvalidOperation);
            }

            return await RunTaskWithErrorHandlingAsync(async () =>
            {
                status = Status.FinishingHostingRoom;
                await RemoveHostingRoomImplAsync();
                status = Status.SearchingRoom;
                hostingRoomInfo = null;
            }, () => status = Status.HostingRoom);
        }

        /// <summary>
        /// Join to a room on the server.
        /// </summary>
        /// <param name="roomId"></param>
        /// <param name="password"></param>
        /// <param name="callback"></param>
        /// <returns></returns>
        public void JoinRoom(uint roomId, string password = "",
            Action<ErrorInfo, JoinRoomCallbackArgs> callback = null)
        {
            RunTask(async () => await JoinRoomAsync(roomId, password), callback);
        }

        /// <summary>
        /// Join to a room on the server.
        /// </summary>
        /// <param name="roomId"></param>
        /// <param name="password"></param>
        /// <returns></returns>
        public async Task<(ErrorInfo errorInfo, JoinRoomCallbackArgs result)> JoinRoomAsync(uint roomId,
            string password = "")
        {
            if (status != Status.SearchingRoom)
            {
                Debug.LogError("The operation is valid when searching room.");
                return (new ErrorInfo(ClientErrorCode.InvalidOperation), default);
            }

            return await RunTaskWithErrorHandlingAsync(async () =>
            {
                status = Status.StartingJoiningRoom;
                var roomHostClientAddress = await JoinRoomImplAsync(roomId, password);
                Disconnect();
                return new JoinRoomCallbackArgs(roomHostClientAddress);
            }, () => status = Status.SearchingRoom);
        }

        #endregion

        #region OtherPublicMethods

        public void Dispose()
        {
            _client.Dispose();
            Reset();
        }

        #endregion

        #region PrivateFields

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

        #endregion

        #region UnityEvents

        private void Awake()
        {
            _client = new MatchMakerClient((int)(_serverCommunicationTimeOutSeconds * 1000), new UnityLogger());
            _client.Logger.Enabled = _enableDebugLog;
        }

        private void OnDestroy()
        {
            _client.Dispose();
        }

        #endregion

        #region OperationHelpers

        private async Task<ErrorInfo> RunTaskWithErrorHandlingAsync(Func<Task> task, Action errorHandler)
        {
            try
            {
                await task();
                return new ErrorInfo(ClientErrorCode.Ok);
            }
            catch (Exception e)
            {
                Debug.LogException(e);
                errorHandler?.Invoke();
                switch (e)
                {
                    case ClientErrorException ce:
                        if (ce.ClientErrorCode == ClientErrorCode.ConnectionClosed)
                        {
                            Disconnect();
                        }

                        return new ErrorInfo(e);
                    default:
                        return new ErrorInfo(e);
                }
            }
        }

        private async Task<(ErrorInfo errorInfo, T result)> RunTaskWithErrorHandlingAsync<T>(Func<Task<T>> task,
            Action errorHandler, T defaultResult = default)
        {
            try
            {
                var result = await task();
                return (new ErrorInfo(ClientErrorCode.Ok), result);
            }
            catch (Exception e)
            {
                Debug.LogException(e);
                errorHandler?.Invoke();
                switch (e)
                {
                    case ClientErrorException ce:
                        if (!ce.ClientErrorCode.IsContinuable())
                        {
                            Disconnect();
                        }

                        return (new ErrorInfo(e), defaultResult);
                    default:
                        return (new ErrorInfo(e), defaultResult);
                }
            }
        }

        private void RunTask(Func<Task<ErrorInfo>> task, Action<ErrorInfo> callback)
        {
            async Task Wrapper()
            {
                var errorInfo = await task();
                callback?.Invoke(errorInfo);
            }

            _task = Task.Run(Wrapper);
        }

        private void RunTask<T>(Func<Task<(ErrorInfo, T)>> task, Action<ErrorInfo, T> callback)
        {
            async Task Wrapper()
            {
                var (errorInfo, result) = await task();
                callback?.Invoke(errorInfo, result);
            }

            _task = Task.Run(Wrapper);
        }

        #endregion

        #region OperationImplementations

        private async Task<PlayerFullName> ConnectImplAsync(string playerName)
        {
            return await _client.ConnectAsync(_serverAddress, _serverPort, playerName);
        }

        private async Task GetRoomGroupListImplAsync()
        {
            _roomGroupInfoList = (await _client.GetRoomGroupListAsync()).Select(result => new RoomGroupInfo(result))
                .ToList();
        }

        private async Task CreateRoomImplAsync(byte maxPlayerCount, bool isPublic = true,
            string password = "")
        {
            await _client.CreateRoomAsync(roomGroupIndex, maxPlayerCount, _gameDefaultPort, isPublic,
                password);
            hostingRoomInfo = new HostingRoomInfo(_client.HostingRoomGroupIndex, isPublic, _client.HostingRoomId,
                maxPlayerCount);
        }

        private async Task<(bool isDefaultPortUsed, ushort privatePort, ushort publicPort)>
            CreateRoomWithCreatingPortMappingImplAsync(byte maxPlayerCount, bool isPublic = true, string password = "")
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

        private async Task<IPEndPoint> JoinRoomImplAsync(uint roomId, string password = "")
        {
            var roomGameHostEndPoint = await _client.JoinRoomAsync(roomGroupIndex, roomId, password);
            return roomGameHostEndPoint;
        }

        private async Task UpdateHostingRoomStatusImplAsync(RoomStatus roomStatus,
            bool updateCurrentPlayerCount = false,
            byte currentPlayerCount = 0)
        {
            await _client.UpdateHostingRoomStatusAsync(roomStatus, updateCurrentPlayerCount, currentPlayerCount);
        }

        private async Task RemoveHostingRoomImplAsync()
        {
            await _client.RemoveHostingRoomAsync();
        }

        private async Task<(byte totalRoomCount, byte matchedRoomCount, byte startIndex, IReadOnlyList<RoomInfo>
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

        #endregion

        #region OtherPrivateMethods

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

        #endregion
    }
}