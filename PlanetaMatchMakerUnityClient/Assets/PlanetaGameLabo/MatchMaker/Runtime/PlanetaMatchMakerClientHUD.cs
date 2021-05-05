using System;
using System.Collections;
using System.Linq;
using System.Net;
using UnityEngine;

namespace PlanetaGameLabo.MatchMaker
{
    [RequireComponent(typeof(PlanetaMatchMakerClient)),
     AddComponentMenu("PlanetaGameLabo/MatchMaker/PlanetaMatchMakerClientHUD"),
     DisallowMultipleComponent]
    public sealed class PlanetaMatchMakerClientHUD : MonoBehaviour
    {
        [SerializeField] private Vector2Int _position;
        [SerializeField] private float _roomListUpdateIntervalSeconds = 3;

        private PlanetaMatchMakerClient _client;
        private RoomInfo[] _roomList = { };
        private bool _isErrorOccured;
        private string _errorMessage;
        private bool _isJoinedRoom;
        private IPEndPoint _joinedRoomHost;

        private string _playerName = "test";
        private string _roomPassword = "";
        private byte _roomMaxPlayerCount = 8;

        private string _roomSearchName = "";
        private ushort _roomSearchTag = PlayerFullName.NotAssignedTag;
        private bool _searchPublicRoom = true;
        private bool _searchPrivateRoom = true;
        private bool _searchOpenRoom = true;
        private bool _searchClosedRoom = true;

        private bool _openRoom = true;
        private byte _roomCurrentPlayerCount;

        private void Awake()
        {
            _client = GetComponent<PlanetaMatchMakerClient>();
        }

        private void OnGUI()
        {
            var position = new Vector2(_position.x, _position.y);
            var size = new Vector2(240, 9999);
            using (var _ = new GUILayout.AreaScope(new Rect(position, size)))
            {
                GUI.Box(new Rect(Vector2.zero, size), "");
                switch (_client.status)
                {
                    case PlanetaMatchMakerClient.Status.Disconnected:
                        GUILayout.Label("Player Name");
                        _playerName = GUILayout.TextField(_playerName);
                        if (GUILayout.Button("Connect"))
                        {
                            _client.Connect(_playerName);
                        }

                        if (_isJoinedRoom)
                        {
                            GUILayout.Label($"Joined Room Host: {_joinedRoomHost}");
                        }

                        break;
                    case PlanetaMatchMakerClient.Status.Connecting:
                        GUILayout.Label($"Connecting..");
                        break;
                    case PlanetaMatchMakerClient.Status.SearchingRoom:
                        GUILayout.Label($"Player Full Name: {_client.playerFullName.GenerateFullName()}");

                        // Display room list
                        GUILayout.Label($"===Room List Setting===");
                        _searchPublicRoom = GUILayout.Toggle(_searchPublicRoom, "Search Public");
                        _searchPrivateRoom = GUILayout.Toggle(_searchPrivateRoom, "Search Private");
                        _searchOpenRoom = GUILayout.Toggle(_searchOpenRoom, "Search Open");
                        _searchClosedRoom = GUILayout.Toggle(_searchClosedRoom, "Search Closed");
                        GUILayout.Label("Search Name");
                        _roomSearchName = GUILayout.TextField(_roomSearchName);
                        GUILayout.Label("Search Tag");
                        ushort.TryParse(GUILayout.TextField(_roomSearchTag.ToString()),
                            out _roomSearchTag);
                        GUILayout.Label($"===Room List===");
                        GUILayout.Label($"Push button to join.");
                        foreach (var room in _roomList)
                        {
                            if (GUILayout.Button(
                                $"{room.hostPlayerFullName}: {room.roomId}({room.currentPlayerCount}/{room.maxPlayerCount})\n[{room.settingFlags}]\n@{room.createDatetime}")
                            )
                            {
                                JoinRoom(room.roomId);
                            }
                        }

                        GUILayout.Label($"===Selected or Create Room Info===");
                        GUILayout.Label("Room Password");
                        _roomPassword = GUILayout.TextField(_roomPassword);
                        GUILayout.Label("Max Player Count");
                        byte.TryParse(GUILayout.TextField(_roomMaxPlayerCount.ToString()),
                            out _roomMaxPlayerCount);

                        GUILayout.Label($"===Operations===");
                        if (GUILayout.Button("Create"))
                        {
                            HostRoom();
                        }

                        if (GUILayout.Button("CreateWithPM"))
                        {
                            HostRoomWithCreatingPortMapping();
                        }

                        if (GUILayout.Button("Disconnect"))
                        {
                            Close();
                        }

                        break;
                    case PlanetaMatchMakerClient.Status.StartingHostingRoom:
                        GUILayout.Label($"Creating hosting room...");
                        break;
                    case PlanetaMatchMakerClient.Status.HostingRoom:
                        GUILayout.Label($"Player Full Name: {_client.playerFullName.GenerateFullName()}");
                        GUILayout.Label("Current Player Count");
                        byte.TryParse(GUILayout.TextField(_roomCurrentPlayerCount.ToString()),
                            out _roomCurrentPlayerCount);
                        _openRoom = GUILayout.Toggle(_openRoom, "Open Room");
                        if (GUILayout.Button("Update"))
                        {
                            UpdateHostingRoomStatus();
                        }

                        if (GUILayout.Button("Remove"))
                        {
                            RemoveHostingRoom();
                        }

                        if (GUILayout.Button("Disconnect"))
                        {
                            Close();
                        }

                        break;
                    case PlanetaMatchMakerClient.Status.FinishingHostingRoom:
                        GUILayout.Label($"Removing hosting room...");
                        break;
                    case PlanetaMatchMakerClient.Status.StartingJoiningRoom:
                        GUILayout.Label($"Joining to room...");
                        break;
                    default:
                        throw new ArgumentOutOfRangeException();
                }

                if (_isErrorOccured)
                {
                    GUILayout.Label(_errorMessage);
                }
            }
        }

        private void OnEnable()
        {
            StartCoroutine(CrtUpdate());
        }

        private IEnumerator CrtUpdate()
        {
            while (true)
            {
                if (_client.status == PlanetaMatchMakerClient.Status.SearchingRoom)
                {
                    RequestRoomList();
                }

                yield return new WaitForSeconds(_roomListUpdateIntervalSeconds);
            }
        }


        private void Close()
        {
            if (_client.connected)
            {
                _client.Disconnect();
            }

            _isErrorOccured = false;
            _isJoinedRoom = false;
            _roomList = new RoomInfo[] { };
        }

        private void RequestRoomList()
        {
            _isErrorOccured = false;
            var searchTargetFlags = RoomSearchTargetFlag.None;
            if (_searchPublicRoom)
            {
                searchTargetFlags |= RoomSearchTargetFlag.PublicRoom;
            }

            if (_searchPrivateRoom)
            {
                searchTargetFlags |= RoomSearchTargetFlag.PrivateRoom;
            }

            if (_searchOpenRoom)
            {
                searchTargetFlags |= RoomSearchTargetFlag.OpenRoom;
            }

            if (_searchClosedRoom)
            {
                searchTargetFlags |= RoomSearchTargetFlag.ClosedRoom;
            }

            _client.RequestRoomList(0, 100,
                RoomDataSortKind.NameAscending, searchTargetFlags, _roomSearchName, _roomSearchTag, (errorInfo, args) =>
                {
                    if (errorInfo)
                    {
                        _roomList = args.roomInfoList.ToArray();
                        return;
                    }

                    _isErrorOccured = true;
                    _errorMessage = errorInfo.message;
                });
        }

        private void HostRoom()
        {
            _isErrorOccured = false;
            _client.HostRoom(_roomMaxPlayerCount, _roomPassword, (errorInfo, args) =>
            {
                if (errorInfo)
                {
                    return;
                }

                _isErrorOccured = true;
                _errorMessage = errorInfo.message;
            });
        }

        private void HostRoomWithCreatingPortMapping()
        {
            _isErrorOccured = false;
            _client.HostRoomWithCreatingPortMapping(_roomMaxPlayerCount, _roomPassword,
                (errorInfo, args) =>
                {
                    if (errorInfo)
                    {
                        return;
                    }

                    _isErrorOccured = true;
                    _errorMessage = errorInfo.message;
                });
        }

        private void JoinRoom(uint roomId)
        {
            try
            {
                _isErrorOccured = false;
                _client.JoinRoom(roomId, _roomPassword, (errorInfo, args) =>
                {
                    if (errorInfo)
                    {
                        Close();
                        _isJoinedRoom = true;
                        _joinedRoomHost = args.roomGameHostEndPoint;
                        return;
                    }

                    _isErrorOccured = true;
                    _errorMessage = errorInfo.message;
                });
            }
            catch (InvalidOperationException e)
            {
                Debug.LogException(e);
                _isErrorOccured = true;
                _errorMessage = e.Message;
            }
        }

        private void RemoveHostingRoom()
        {
            _isErrorOccured = false;
            _client.RemoveHostingRoom(errorInfo =>
            {
                if (errorInfo)
                {
                    return;
                }

                _isErrorOccured = true;
                _errorMessage = errorInfo.message;
            });
        }

        private void UpdateHostingRoomStatus()
        {
            _isErrorOccured = false;
            _client.ChangeOpenOrCloseHostingRoom(_openRoom, _roomCurrentPlayerCount, errorInfo =>
            {
                if (errorInfo)
                {
                    return;
                }

                _isErrorOccured = true;
                _errorMessage = errorInfo.message;
            });
        }
    }
}