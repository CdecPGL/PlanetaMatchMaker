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

        /// <summary>
        /// A status of unity client.
        /// </summary>
        public enum Status
        {
            Disconnected, Connecting, SearchingRoom, StartingHostingRoom, HostingRoom, FinishingHostingRoom,
            StartingJoiningRoom
        }

        /// <summary>
        /// A struct to hold error information.
        /// </summary>
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
        /// Singleton instance. This is valid when isSingleton is true.
        /// </summary>
        public static PlanetaMatchMakerClient singleton { get; private set; }

        /// <summary>
        /// A status of client.
        /// </summary>
        public Status status
        {
            get
            {
                if (_status != Status.Disconnected && !_client.Connected)
                {
                    Reset();
                }

                return _status;
            }
            private set => _status = value;
        }

        /// <summary>
        /// true if connected to the server.
        /// </summary>
        public bool connected => _client?.Connected ?? false;

        /// <summary>
        /// An address of matching server.
        /// </summary>
        public string serverAddress
        {
            get => _serverAddress;
            set => _serverAddress = value;
        }

        /// <summary>
        /// A port of matching server.
        /// </summary>
        public ushort serverPort
        {
            get => _serverPort;
            set => _serverPort = value;
        }

        /// <summary>
        /// A transport protocol your game is using.
        /// </summary>
        public TransportProtocol gameTransportProtocol
        {
            get => _gameTransportProtocol;
            set => _gameTransportProtocol = value;
        }

        /// <summary>
        /// A port to host game which we try to use first.
        /// </summary>
        public ushort gameDefaultPort
        {
            get => _gameDefaultPort;
            set => _gameDefaultPort = value;
        }

        /// <summary>
        /// A range of port.
        /// </summary>
        [Serializable]
        public struct PortRange
        {
            public ushort startPort
            {
                get => _startPort;
                set => _startPort = value;
            }

            public ushort endPort
            {
                get => _endPort;
                set => _endPort = value;
            }

            [SerializeField] private ushort _startPort;
            [SerializeField] private ushort _endPort;
        }

        /// <summary>
        /// A port to host game which we try to select a port to use from when default game port is not available.
        /// </summary>
        public List<PortRange> gamePortCandidates
        {
            get => _gamePortCandidates;
            set => _gamePortCandidates = value;
        }

        /// <summary>
        /// A hosting room information.
        /// This is valid when the client is hosting room.
        /// </summary>
        public IReadOnlyHostingRoomInfo hostingRoomInfo => _hostingRoomInfo;

        /// <summary>
        /// True if the client is hosting room.
        /// </summary>
        public bool isHostingRoom => _client.IsHostingRoom;

        public PlayerFullName playerFullName => _client.PlayerFullName;

        #endregion

        #region Operations

        /// <summary>
        /// Connect to the matching server
        /// </summary>
        /// <param name="playerName"></param>
        /// <param name="callback"></param>
        public void Connect(string playerName, Action<ErrorInfo, ConnectResult> callback = null)
        {
            RunTask(async () => await ConnectAsync(playerName), callback);
        }

        /// <summary>
        /// Connect to the matching server
        /// </summary>
        /// <param name="playerName"></param>
        public async Task<(ErrorInfo errorInfo, ConnectResult result)> ConnectAsync(string playerName)
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
                return new ConnectResult(returnedPlayerFullName);
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
        /// <param name="searchTag"></param>
        /// <param name="callback"></param>
        public void RequestRoomList(ushort resultStartIndex, ushort resultCount, RoomDataSortKind sortKind,
            RoomSearchTargetFlag targetFlags, string searchName = "", ushort searchTag = 0,
            Action<ErrorInfo, RequestRoomListResult> callback = null)
        {
            RunTask(
                async () => await RequestRoomListAsync(resultStartIndex, resultCount, sortKind, targetFlags,
                    searchName, searchTag), callback);
        }

        /// <summary>
        /// Request room list to the server.
        /// </summary>
        /// <param name="resultStartIndex"></param>
        /// <param name="resultCount"></param>
        /// <param name="sortKind"></param>
        /// <param name="targetFlags"></param>
        /// <param name="searchName"></param>
        /// <param name="searchTag"></param>
        public async Task<(ErrorInfo errorInfo, RequestRoomListResult result)> RequestRoomListAsync(
            ushort resultStartIndex, ushort resultCount, RoomDataSortKind sortKind,
            RoomSearchTargetFlag targetFlags, string searchName = "", ushort searchTag = 0)
        {
            if (status != Status.SearchingRoom)
            {
                Debug.LogError("The operation is valid when searching room.");
                return (new ErrorInfo(ClientErrorCode.InvalidOperation), default);
            }

            return await RunTaskWithErrorHandlingAsync(async () =>
                {
                    var (totalRoomCount, _, startIndex, roomInfoList) = await GetRoomListAsync(resultStartIndex,
                        resultCount, sortKind, targetFlags, searchName, searchTag);
                    return new RequestRoomListResult(totalRoomCount, startIndex,
                        roomInfoList);
                },
                () => { });
        }

        /// <summary>
        /// Create and host new room to the server.
        /// </summary>
        /// <param name="maxPlayerCount"></param>
        /// <param name="password"></param>
        /// <param name="callback"></param>
        /// <returns></returns>
        public void HostRoom(byte maxPlayerCount, string password = "",
            Action<ErrorInfo, HostRoomResult> callback = null)
        {
            RunTask(async () => await HostRoomAsync(maxPlayerCount, password), callback);
        }

        /// <summary>
        /// Create and host new room to the server.
        /// </summary>
        /// <param name="maxPlayerCount"></param>
        /// <param name="password"></param>
        /// <returns></returns>
        public async Task<(ErrorInfo errorInfo, HostRoomResult result)> HostRoomAsync(byte maxPlayerCount,
            string password = "")
        {
            if (status != Status.SearchingRoom)
            {
                Debug.LogError("The operation is valid when searching room.");
                return (new ErrorInfo(ClientErrorCode.InvalidOperation), default);
            }

            return await RunTaskWithErrorHandlingAsync(async () =>
            {
                status = Status.StartingHostingRoom;
                var result = await CreateRoomImplAsync(maxPlayerCount, password);
                status = Status.HostingRoom;
                return result;
            }, () => status = Status.SearchingRoom);
        }

        /// <summary>
        /// Create and host new room to the server with trying to create port mapping.
        /// </summary>
        /// <param name="maxPlayerCount"></param>
        /// <param name="password"></param>
        /// <param name="callback"></param>
        /// <returns></returns>
        public void HostRoomWithCreatingPortMapping(byte maxPlayerCount, string password = "",
            Action<ErrorInfo, HostRoomWithCreatingPortMappingResult> callback = null)
        {
            RunTask(async () => await HostRoomWithCreatingPortMappingAsync(maxPlayerCount, password), callback);
        }

        /// <summary>
        /// Create and host new room to the server with trying to create port mapping.
        /// </summary>
        /// <param name="maxPlayerCount"></param>
        /// <param name="password"></param>
        /// <returns></returns>
        public async Task<(ErrorInfo errorInfo, HostRoomWithCreatingPortMappingResult result)>
            HostRoomWithCreatingPortMappingAsync(byte maxPlayerCount, string password = "")
        {
            if (status != Status.SearchingRoom)
            {
                Debug.LogError("The operation is valid when searching room.");
                return (new ErrorInfo(ClientErrorCode.InvalidOperation), default);
            }

            return await RunTaskWithErrorHandlingAsync(async () =>
            {
                status = Status.StartingHostingRoom;
                var result = await CreateRoomWithCreatingPortMappingImplAsync(maxPlayerCount, password);
                status = Status.HostingRoom;
                return result;
            }, () => status = Status.SearchingRoom);
        }

        /// <summary>
        /// Create and host new room to the server with trying to create port mapping.
        /// </summary>
        /// <param name="connectionEstablishMode"></param>
        /// <param name="externalId"></param>
        /// <param name="maxPlayerCount"></param>
        /// <param name="password"></param>
        /// <param name="callback"></param>
        /// <returns></returns>
        public void HostRoomWithExternalService<T>(GameHostConnectionEstablishMode connectionEstablishMode,
            T externalId, byte maxPlayerCount, string password = "", Action<ErrorInfo, HostRoomResult> callback = null)
        {
            RunTask(
                async () => await HostRoomWithExternalServiceAsync(connectionEstablishMode, externalId, maxPlayerCount,
                    password), callback);
        }

        /// <summary>
        /// Create and host new room to the server with external service to establish connection.
        /// </summary>
        /// <param name="externalId"></param>
        /// <param name="maxPlayerCount"></param>
        /// <param name="password"></param>
        /// <param name="connectionEstablishMode"></param>
        /// <returns></returns>
        public async Task<(ErrorInfo errorInfo, HostRoomResult result)> HostRoomWithExternalServiceAsync<T>(
            GameHostConnectionEstablishMode connectionEstablishMode, T externalId, byte maxPlayerCount,
            string password = "")
        {
            if (status != Status.SearchingRoom)
            {
                Debug.LogError("The operation is valid when searching room.");
                return (new ErrorInfo(ClientErrorCode.InvalidOperation), default);
            }

            return await RunTaskWithErrorHandlingAsync(async () =>
            {
                status = Status.StartingHostingRoom;
                var result = await CreateRoomWithExternalServiceImplAsync(connectionEstablishMode, externalId,
                    maxPlayerCount, password);
                status = Status.HostingRoom;
                return result;
            }, () => status = Status.SearchingRoom);
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
        /// Change open status of hosting room with updating current player count.
        /// </summary>
        /// <param name="currentPlayerCount"></param>
        /// <param name="callback"></param>
        public void UpdateCurrentPlayerCountOfHostingRoom(byte currentPlayerCount, Action<ErrorInfo> callback = null)
        {
            RunTask(
                async () => await UpdateCurrentPlayerCountOfHostingRoomAsync(currentPlayerCount), callback);
        }

        /// <summary>
        /// Change open status of hosting room with updating current player count.
        /// </summary>
        /// <param name="currentPlayerCount"></param>
        public async Task<ErrorInfo> UpdateCurrentPlayerCountOfHostingRoomAsync(byte currentPlayerCount)
        {
            if (status != Status.HostingRoom)
            {
                Debug.LogError("Not hosting room.");
                return new ErrorInfo(ClientErrorCode.InvalidOperation);
            }

            return await RunTaskWithErrorHandlingAsync(
                async () => await UpdateHostingRoomCurrentPlayerCountImplAsync(currentPlayerCount), () => { });
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
                _hostingRoomInfo = null;
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
            Action<ErrorInfo, JoinRoomResult> callback = null)
        {
            RunTask(async () => await JoinRoomAsync(roomId, password), callback);
        }

        /// <summary>
        /// Join to a room on the server.
        /// </summary>
        /// <param name="roomId"></param>
        /// <param name="password"></param>
        /// <returns></returns>
        public async Task<(ErrorInfo errorInfo, JoinRoomResult result)> JoinRoomAsync(uint roomId,
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
                return new JoinRoomResult(roomHostClientAddress);
            }, () => status = Status.SearchingRoom);
        }

        /// <summary>
        /// Join to a room with external service on the server.
        /// </summary>
        /// <param name="connectionEstablishMode"></param>
        /// <param name="roomId"></param>
        /// <param name="password"></param>
        /// <param name="callback"></param>
        /// <returns></returns>
        public void JoinRoomWithExternalService(GameHostConnectionEstablishMode connectionEstablishMode, uint roomId,
            string password = "", Action<ErrorInfo, JoinRoomWithExternalServiceResult> callback = null)
        {
            RunTask(async () => await JoinRoomWithExternalServiceAsync(connectionEstablishMode, roomId, password),
                callback);
        }

        /// <summary>
        /// Join to a room with external service on the server.
        /// </summary>
        /// <param name="connectionEstablishMode"></param>
        /// <param name="roomId"></param>
        /// <param name="password"></param>
        /// <returns></returns>
        public async Task<(ErrorInfo errorInfo, JoinRoomWithExternalServiceResult result)>
            JoinRoomWithExternalServiceAsync(
                GameHostConnectionEstablishMode connectionEstablishMode, uint roomId, string password = "")
        {
            if (status != Status.SearchingRoom)
            {
                Debug.LogError("The operation is valid when searching room.");
                return (new ErrorInfo(ClientErrorCode.InvalidOperation), default);
            }

            return await RunTaskWithErrorHandlingAsync(async () =>
            {
                status = Status.StartingJoiningRoom;
                var result = await JoinRoomWithExternalServiceImplAsync(connectionEstablishMode, roomId, password);
                Disconnect();
                return result;
            }, () => status = Status.SearchingRoom);
        }

        /// <summary>
        /// Check if the client is connected to client and update status.
        /// </summary>
        /// <returns></returns>
        public bool CheckConnectionAndUpdateStatus()
        {
            var task = CheckConnectionAndUpdateStatusAsync();
            task.Wait();
            return task.Result;
        }

        /// <summary>
        /// Check if the client is connected to client and update status.
        /// </summary>
        /// <returns></returns>
        public async Task<bool> CheckConnectionAndUpdateStatusAsync()
        {
            if (!_client.Connected)
            {
                return false;
            }

            try
            {
                await _client.NoticeAliveToTheServerAsync();
                return true;
            }
            catch (ClientErrorException e)
            {
                return e.ClientErrorCode.IsContinuable();
            }
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

        [SerializeField, Tooltip("Don't destroy the game object which is attached this component on load if true")]
        private bool _dontDestroyOnLoad;

        [SerializeField, Tooltip("the game object which is attached this component become singleton if true.")]
        private bool _isSingleton;

        [SerializeField, Tooltip("IP Address of Match Making Server")]
        private string _serverAddress = "127.0.0.1";

        [SerializeField, Tooltip("Port of Match Making Server")]
        private ushort _serverPort = 57000;


        [SerializeField, Tooltip("Timeout seconds to send and receive data between match making server")]
        private float _serverCommunicationTimeOutSeconds = 10;

        [SerializeField, Tooltip("The interval time to send alive notice to the server to avoid timeout.")]
        private int _keepAliveNoticeIntervalSeconds = 30;

        [Header("===Builtin Mode===")] [SerializeField, Tooltip("A transport protocol used for game")]
        private TransportProtocol _gameTransportProtocol;

        [SerializeField, Tooltip("Port of Game Host")]
        private ushort _gameDefaultPort = 52000;

        [SerializeField, Tooltip("Candidates of ports used for game port")]
        private List<PortRange> _gamePortCandidates;

        [SerializeField, Tooltip("Timeout seconds to discover NAT device")]
        private float _natDiscoverTimeOutSeconds = 5;

        private MatchMakerClient _client;
        private Status _status = Status.Disconnected;
        private HostingRoomInfo _hostingRoomInfo;

        [Header("===Development===")] [SerializeField, Tooltip("Enable debug log")]
        private bool _enableDebugLog;

        [SerializeField, Tooltip("Threshold of log level to output when debug log is enabled")]
        private LogLevel _debugLogLevel = LogLevel.Info;

        #endregion

        #region UnityEvents

        private void Awake()
        {
            _client = new MatchMakerClient((int)(_serverCommunicationTimeOutSeconds * 1000),
                _keepAliveNoticeIntervalSeconds, new UnityLogger(_debugLogLevel));
            _client.Logger.Enabled = _enableDebugLog;
            if (_dontDestroyOnLoad)
            {
                DontDestroyOnLoad(gameObject);
            }

            if (!_isSingleton)
            {
                return;
            }

            if (singleton)
            {
                Destroy(gameObject);
            }
            else
            {
                singleton = this;
            }
        }

        private void OnDestroy()
        {
            _client.Dispose();

            if (!_isSingleton)
            {
                return;
            }

            if (singleton == this)
            {
                singleton = null;
            }
        }

        #endregion

        #region OperationHelpers

        internal async Task<ErrorInfo> RunTaskWithErrorHandlingAsync(Func<Task> task, Action errorHandler)
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

        internal async Task<(ErrorInfo errorInfo, T result)> RunTaskWithErrorHandlingAsync<T>(Func<Task<T>> task,
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

        internal static void RunTask(Func<Task<ErrorInfo>> task, Action<ErrorInfo> callback)
        {
            async Task Wrapper()
            {
                var errorInfo = await task();
                callback?.Invoke(errorInfo);
            }

            Task.Run(Wrapper);
        }

        internal static void RunTask<T>(Func<Task<(ErrorInfo, T)>> task, Action<ErrorInfo, T> callback)
        {
            async Task Wrapper()
            {
                var (errorInfo, result) = await task();
                callback?.Invoke(errorInfo, result);
            }

            Task.Run(Wrapper);
        }

        #endregion

        #region OperationImplementations

        private async Task<PlayerFullName> ConnectImplAsync(string playerName)
        {
            return await _client.ConnectAsync(_serverAddress, _serverPort, playerName);
        }

        private async Task<HostRoomResult> CreateRoomImplAsync(byte maxPlayerCount,
            string password = "")
        {
            var createRoomResult = await _client.CreateRoomAsync(maxPlayerCount, _gameDefaultPort, password);
            _hostingRoomInfo = new HostingRoomInfo(createRoomResult, password);
            return new HostRoomResult(_hostingRoomInfo);
        }

        private async Task<HostRoomWithCreatingPortMappingResult>
            CreateRoomWithCreatingPortMappingImplAsync(byte maxPlayerCount, string password = "")
        {
            var gameDefaultPortSnapshot = _gameDefaultPort;
            var result = await _client.CreateRoomWithCreatingPortMappingAsync(maxPlayerCount,
                _gameTransportProtocol, GenerateGamePortCandidateList(), gameDefaultPortSnapshot,
                (int)(_natDiscoverTimeOutSeconds * 1000), password);
            _hostingRoomInfo = new HostingRoomInfo(result.CreteRoomResult, password);
            return new HostRoomWithCreatingPortMappingResult(_hostingRoomInfo, result.IsDefaultPortUsed,
                result.IsDefaultPortUsed ? gameDefaultPortSnapshot : result.UsedPrivatePortFromCandidates,
                result.IsDefaultPortUsed ? gameDefaultPortSnapshot : result.UsedPublicPortFromCandidates);
        }

        private async Task<HostRoomResult> CreateRoomWithExternalServiceImplAsync<T>(
            GameHostConnectionEstablishMode connectionEstablishMode, T externalId, byte maxPlayerCount,
            string password = "")
        {
            CreateRoomResult createRoomResult;
            switch (externalId)
            {
                case byte[] externalIdByteArray:
                    createRoomResult = await _client.CreateRoomWithExternalServiceAsync(maxPlayerCount,
                        connectionEstablishMode, externalIdByteArray, password);
                    break;
                default:
                    createRoomResult = await _client.CreateRoomWithExternalServiceAsync(maxPlayerCount,
                        connectionEstablishMode, externalId, password);
                    break;
            }

            _hostingRoomInfo = new HostingRoomInfo(createRoomResult, password);
            return new HostRoomResult(_hostingRoomInfo);
        }

        private async Task<IPEndPoint> JoinRoomImplAsync(uint roomId, string password = "")
        {
            return await _client.JoinRoomAsync(roomId, password);
        }

        private async Task<JoinRoomWithExternalServiceResult> JoinRoomWithExternalServiceImplAsync(
            GameHostConnectionEstablishMode connectionEstablishMode, uint roomId, string password = "")
        {
            return await _client.JoinRoomWithExternalServiceAsync(roomId, connectionEstablishMode, password);
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

        private async Task UpdateHostingRoomCurrentPlayerCountImplAsync(byte currentPlayerCount)
        {
            await _client.UpdateHostingRoomCurrentPlayerCountAsync(currentPlayerCount);
        }

        private async Task<(ushort totalRoomCount, ushort matchedRoomCount, ushort startIndex, IReadOnlyList<RoomInfo>
                roomInfoList)>
            GetRoomListAsync(ushort resultStartIndex, ushort resultCount, RoomDataSortKind sortKind,
                RoomSearchTargetFlag targetFlags, string searchName, ushort searchTag)
        {
            var (totalRoomCount, matchedRoomCount, roomInfoList) = await _client.GetRoomListAsync(
                resultStartIndex, resultCount, sortKind, targetFlags, searchName, searchTag);
            return (totalRoomCount, resultStartIndex, matchedRoomCount,
                roomInfoList.Select(result => new RoomInfo(result)).ToList().AsReadOnly());
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
            _hostingRoomInfo = null;
        }

        #endregion
    }
}