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
        private bool _isJoinedRoom;
        private ClientAddress _joinedRoomHost;
        private Task _currentTask;

        private string _roomName = "";
        private string _roomPassword = "";
        private byte _roomMaxPlayerCount;
        private bool _createPublicRoom = true;

        private string _searchRoomName = "";
        private bool _searchPublicRoom = true;
        private bool _searchPrivateRoom = true;
        private bool _searchOpenRoom = true;
        private bool _searchClosedRoom = true;

        private bool _openRoom = true;

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
                if (_client.connected)
                {
                    if (_client.isHostingRoom)
                    {
                        GUILayout.Label($"Room Name: {_roomName}");
                        GUILayout.Label("Max Player Count");
                        byte.TryParse(GUILayout.TextField(_roomMaxPlayerCount.ToString()), out _roomMaxPlayerCount);
                        _openRoom = GUILayout.Toggle(_openRoom, "Open Room");
                        if (GUILayout.Button("Update") && !IsTaskRunning())
                        {
                            _currentTask = UpdateHostingRoomStatusAsync();
                        }

                        if (GUILayout.Button("Remove") && !IsTaskRunning())
                        {
                            _currentTask = RemoveHostingRoomAsync();
                        }
                    }
                    else
                    {
                        // Display room group list
                        GUILayout.Label("===Room Group List===");
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
                        GUILayout.Label($"===Room List Setting===");
                        _searchPublicRoom = GUILayout.Toggle(_searchPublicRoom, "Search Public");
                        _searchPrivateRoom = GUILayout.Toggle(_searchPrivateRoom, "Search Private");
                        _searchOpenRoom = GUILayout.Toggle(_searchOpenRoom, "Search Open");
                        _searchClosedRoom = GUILayout.Toggle(_searchClosedRoom, "Search Closed");
                        GUILayout.Label("Search Name");
                        _searchRoomName = GUILayout.TextField(_searchRoomName);
                        GUILayout.Label($"===Room List in Group {_selectedRoomGroupIndex}===");
                        foreach (var room in _roomList)
                        {
                            GUILayout.Label(
                                $"{room.RoomId}: {room.Name} ({room.CurrentPlayerCount}/{room.MaxPlayerCount}) [{room.SettingFlags}] @{room.CreateDatetime}");
                        }

                        GUILayout.Label($"===Selected or Create Room Info===");
                        GUILayout.Label("Room Name");
                        _roomName = GUILayout.TextField(_roomName);
                        GUILayout.Label("Room Password");
                        _roomPassword = GUILayout.TextField(_roomPassword);
                        GUILayout.Label("Max Player Count");
                        byte.TryParse(GUILayout.TextField(_roomMaxPlayerCount.ToString()),
                            out _roomMaxPlayerCount);
                        _createPublicRoom = GUILayout.Toggle(_createPublicRoom, "Create Public Room");

                        GUILayout.Label($"===Operations===");
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

                var result = await _client.GetRoomListAsync(_selectedRoomGroupIndex, 0, 100,
                    RoomDataSortKind.NameAscending, searchTargetFlags, _searchRoomName);
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
                await _client.CreateRoomAsync(_selectedRoomGroupIndex, _roomName, _roomMaxPlayerCount,
                    _createPublicRoom, _roomPassword);
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
                var roomId = _roomList.First(r => r.Name == _roomName).RoomId;
                _joinedRoomHost = await _client.JoinRoomAsync(_selectedRoomGroupIndex, roomId, _roomPassword);
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

        private async Task UpdateHostingRoomStatusAsync()
        {
            _isErrorOccured = false;
            try
            {
                await _client.UpdateHostingRoomStatusAsync(_openRoom ? RoomStatus.Open : RoomStatus.Close);
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