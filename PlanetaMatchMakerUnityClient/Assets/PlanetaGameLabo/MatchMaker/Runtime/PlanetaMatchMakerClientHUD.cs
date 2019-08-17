using System;
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

        private void Awake()
        {
            _client = GetComponent<PlanetaMatchMakerClient>();
        }

        private void OnGUI()
        {
            using (var _ = new GUILayout.AreaScope(new Rect(10 + _position.x, 40 + _position.y, 215, 9999)))
            {
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
                        GUILayout.Label($"Room List in Group {_selectedRoomGroupIndex}");
                        foreach (var room in _roomList)
                        {
                            GUILayout.Label(
                                $"{room.RoomId}: {room.Name} ({room.CurrentPlayerCount}/{room.MaxPlayerCount}) [{room.Flags}] @{room.CreateDatetime}");
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
                var result = await _client.GetRoomListAsync(_selectedRoomGroupIndex, 0, 100,
                    RoomDataSortKind.NameAscending,
                    byte.MaxValue);
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