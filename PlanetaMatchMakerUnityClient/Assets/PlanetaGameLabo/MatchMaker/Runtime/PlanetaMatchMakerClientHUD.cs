using System;
using System.Collections;
using System.Linq;
using System.Threading.Tasks;
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
        private RoomGroupResult[] _roomGroupList = { };
        private RoomResult[] _roomList = { };
        private bool _isErrorOccured;
        private string _errorMessage;
        private byte _selectedRoomGroupIndex;
        private string _selectedRoomName = "";
        private bool _isJoinedRoom;
        private ClientAddress _joinedRoomHost;
        private Task _currentTask;
        private byte _maxPlayerCount;
        private string searchRoomName = "";
        private bool searchPublicRoom = true;
        private bool searchPrivateRoom = true;
        private bool searchOpenRoom = true;
        private bool searchClosedRoom = true;

        private void Awake()
        {
            _client = GetComponent<PlanetaMatchMakerClient>();
        }

        private void OnGUI()
        {
            var position = new Vector2( _position.x, _position.y);
            var size = new Vector2(215, 9999);
            using (var _ = new GUILayout.AreaScope(new Rect(position, size)))
            {
                GUI.Box(new Rect(Vector2.zero, size), "");
                if (_client.connected)
                {
                    if (_client.isHostingRoom)
                    {
                        if (GUILayout.Button("Remove") && !IsTaskRunning())
                        {
                            _currentTask = RemoveHostingRoomAsync();
                        }
                    }
                    else
                    {
                        // Display room group list
                        GUILayout.Label("Room Group List");
                        for (var i = 0; i < _roomGroupList.Length; ++i)
                        {
                            GUILayout.Label($"{i}: {_roomGroupList[i].Name}");
                        }

                        GUILayout.Label("Selected Room Group Index");
                        var roomGroupIndexStr = GUILayout.TextField(_selectedRoomGroupIndex.ToString());
                        byte.TryParse(roomGroupIndexStr, out var roomGroupIndex);
                        if (_selectedRoomGroupIndex != roomGroupIndex)
                        {
                            _selectedRoomGroupIndex = roomGroupIndex;
                            _currentTask = GetRoomListAsync();
                        }

                        // Display room list
                        GUILayout.Label($"Room List Setting");
                        searchPublicRoom = GUILayout.Toggle(searchPublicRoom, "Search Public");
                        searchPrivateRoom = GUILayout.Toggle(searchPrivateRoom, "Search Private");
                        searchOpenRoom = GUILayout.Toggle(searchOpenRoom, "Search Open");
                        searchClosedRoom = GUILayout.Toggle(searchClosedRoom, "Search Closed");
                        GUILayout.Label("Search Name");
                        searchRoomName = GUILayout.TextField(searchRoomName);
                        GUILayout.Label($"Room List in Group {_selectedRoomGroupIndex}");
                        foreach (var room in _roomList)
                        {
                            GUILayout.Label(
                                $"{room.RoomId}: {room.Name} ({room.CurrentPlayerCount}/{room.MaxPlayerCount}) [{room.SettingFlags}] @{room.CreateDatetime}");
                        }

                        GUILayout.Label("Selected Room Name");
                        _selectedRoomName = GUILayout.TextField(_selectedRoomName);

                        GUILayout.Label("Max Player Count");
                        byte.TryParse(GUILayout.TextField(_maxPlayerCount.ToString()), out _maxPlayerCount);

                        if (GUILayout.Button("Create") && !IsTaskRunning())
                        {
                            _currentTask = CreateRoomAsync();
                        }

                        if (GUILayout.Button("Join") && !IsTaskRunning())
                        {
                            _currentTask = JoinRoomAsync();
                        }
                    }

                    if (GUILayout.Button("Disconnect") && !IsTaskRunning())
                    {
                        Close();
                    }
                }
                else
                {
                    if (GUILayout.Button("Connect") && !IsTaskRunning())
                    {
                        _currentTask = ConnectAndGetRoomGroup();
                    }

                    if (_isJoinedRoom)
                    {
                        GUILayout.Label($"Joined Room Host: {_joinedRoomHost}");
                    }
                }

                if (IsTaskRunning())
                {
                    GUILayout.Label("Processing...");
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
                if (_client.connected && !_client.isHostingRoom && !IsTaskRunning() &&
                    _selectedRoomGroupIndex < _roomGroupList.Length)
                {
                    _currentTask = GetRoomListAsync();
                }

                yield return new WaitForSeconds(_roomListUpdateIntervalSeconds);
            }
        }

        private async Task ConnectAndGetRoomGroup()
        {
            await ConnectAsync();
            if (_isErrorOccured)
            {
                return;
            }

            await GetRoomGroupListAsync();
            if (_isErrorOccured)
            {
                return;
            }

            _selectedRoomGroupIndex = 0;
            await GetRoomListAsync();
        }

        private async Task ConnectAsync()
        {
            _isErrorOccured = false;
            try
            {
                await _client.ConnectAsync();
            }
            catch (Exception e)
            {
                _isErrorOccured = true;
                _errorMessage = e.Message;
            }
        }

        private void Close()
        {
            _client.Close();
            _isErrorOccured = false;
            _isJoinedRoom = false;
            _roomList = new RoomResult[] { };
            _roomGroupList = new RoomGroupResult[] { };
            _selectedRoomGroupIndex = 0;
        }

        private async Task GetRoomGroupListAsync()
        {
            _isErrorOccured = false;
            try
            {
                _roomGroupList = await _client.GetRoomGroupListAsync();
            }
            catch (Exception e)
            {
                _isErrorOccured = true;
                _errorMessage = e.Message;
            }
        }

        private async Task GetRoomListAsync()
        {
            _isErrorOccured = false;
            try
            {
                var searchTargetFlags = RoomSearchTargetFlag.None;
                if (searchPublicRoom)
                {
                    searchTargetFlags |= RoomSearchTargetFlag.PublicRoom;
                }

                if (searchPrivateRoom)
                {
                    searchTargetFlags |= RoomSearchTargetFlag.PrivateRoom;
                }

                if (searchOpenRoom)
                {
                    searchTargetFlags |= RoomSearchTargetFlag.OpenRoom;
                }

                if (searchClosedRoom)
                {
                    searchTargetFlags |= RoomSearchTargetFlag.ClosedRoom;
                }

                var result = await _client.GetRoomListAsync(_selectedRoomGroupIndex, 0, 100,
                    RoomDataSortKind.NameAscending, searchTargetFlags, searchRoomName);
                _roomList = result.roomInfoList;
            }
            catch (Exception e)
            {
                _isErrorOccured = true;
                _errorMessage = e.Message;
            }
        }

        private async Task CreateRoomAsync()
        {
            _isErrorOccured = false;
            try
            {
                await _client.CreateRoomAsync(_selectedRoomGroupIndex, _selectedRoomName, _maxPlayerCount);
            }
            catch (Exception e)
            {
                _isErrorOccured = true;
                _errorMessage = e.Message;
            }
        }

        private async Task JoinRoomAsync()
        {
            _isErrorOccured = false;
            try
            {
                var roomId = _roomList.First(r => r.Name == _selectedRoomName).RoomId;
                _joinedRoomHost = await _client.JoinRoomAsync(_selectedRoomGroupIndex, roomId);
                Close();
                _isJoinedRoom = true;
            }
            catch (Exception e)
            {
                _isErrorOccured = true;
                _errorMessage = e.Message;
            }
        }

        private async Task RemoveHostingRoomAsync()
        {
            _isErrorOccured = false;
            try
            {
                await _client.RemoveHostingRoomAsync();
            }
            catch (Exception e)
            {
                _isErrorOccured = true;
                _errorMessage = e.Message;
            }
        }

        private bool IsTaskRunning()
        {
            return _currentTask != null && !_currentTask.IsCompleted;
        }
    }
}