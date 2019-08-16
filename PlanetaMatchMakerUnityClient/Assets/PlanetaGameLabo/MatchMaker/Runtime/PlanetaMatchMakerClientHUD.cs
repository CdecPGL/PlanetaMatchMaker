using System.Linq;
using System.Threading.Tasks;
using UnityEngine;

namespace PlanetaGameLabo.MatchMaker {
	[RequireComponent(typeof(PlanetaMatchMakerClient)),
	 AddComponentMenu("PlanetaGameLabo/MatchMaker/PlanetaMatchMakerClientHUD"),
	 DisallowMultipleComponent]
	public sealed class PlanetaMatchMakerClientHUD : MonoBehaviour {
		private PlanetaMatchMakerClient _client;
		private RoomGroupResult[] _roomGroupList;
		private RoomResult[] _roomList;
		private bool _isErrorOccured;
		private string _errorMessage;
		private byte _roomGroupIndex;
		private string _roomName;
		private bool _isJoinedRoom;
		private ClientAddress _joinedRoomHost;
		private Task _currentTask;

		private void Awake() {
			_client = GetComponent<PlanetaMatchMakerClient>();
		}

		private void OnGUI() {
			using (var _ = new GUILayout.AreaScope(new Rect(10, 40, 215, 9999))) {
				if (_client.connected) {
					if (_client.isHostingRoom) {
						if (GUILayout.Button("Remove") && !IsTaskRunning()) {
							_currentTask = RemoveHostingRoomAsync();
						}
					}
					else {
						// Display room group list
						for (var i = 0; i < _roomGroupList.Length; ++i) {
							GUILayout.Label($"{i}: {_roomGroupList[i].Name}");
						}

						var room_group_index_str = GUILayout.TextField("Room Group Index");
						byte.TryParse(room_group_index_str, out var room_group_index);
						if (_roomGroupIndex != room_group_index) {
							_roomGroupIndex = room_group_index;
							_currentTask = GetRoomListAsync();
						}

						// Display room list
						foreach (var room in _roomList) {
							GUILayout.Label(
								$"{room.RoomId}: {room.Name} ({room.CurrentPlayerCount}/{room.MaxPlayerCount}) [{room.Flags}] @{room.CreateDatetime}");
						}

						_roomName = GUILayout.TextField("Room Name");

						if (GUILayout.Button("Create") && !IsTaskRunning()) {
							_currentTask = CreateRoomAsync();
						}

						if (GUILayout.Button("Join") && !IsTaskRunning()) {
							_currentTask = JoinRoomAsync();
						}
					}

					if (GUILayout.Button("Disconnect") && !IsTaskRunning()) {
						Close();
					}
				}
				else {
					if (GUILayout.Button("Connect") && !IsTaskRunning()) {
						_currentTask = ConnectAndGetRoomGroup();
					}

					if (_isJoinedRoom) {
						GUILayout.Label($"Joined Room Host: {_joinedRoomHost}");
					}
				}

				if (IsTaskRunning()) {
					GUILayout.Label("Processing...");
				}

				if (_isErrorOccured) {
					GUILayout.Label(_errorMessage);
				}
			}
		}

		private async Task ConnectAndGetRoomGroup() {
			await ConnectAsync();
			if (_isErrorOccured) {
				return;
			}

			await GetRoomGroupListAsync();
		}

		private async Task ConnectAsync() {
			_isErrorOccured = false;
			try {
				await _client.ConnectAsync();
			}
			catch (ClientErrorException e) {
				_isErrorOccured = true;
				_errorMessage = e.Message;
			}
		}

		private void Close() {
			_client.Close();
			_isErrorOccured = false;
			_isJoinedRoom = false;
			_roomList = null;
			_roomGroupList = null;
			_roomGroupIndex = 0;
		}

		private async Task GetRoomGroupListAsync() {
			_isErrorOccured = false;
			try {
				_roomGroupList = await _client.GetRoomGroupListAsync();
			}
			catch (ClientErrorException e) {
				_isErrorOccured = true;
				_errorMessage = e.Message;
			}
		}

		private async Task GetRoomListAsync() {
			_isErrorOccured = false;
			try {
				var result = await _client.GetRoomListAsync(_roomGroupIndex, 0, 100, RoomDataSortKind.NameAscending,
					byte.MaxValue);
				_roomList = result.roomInfoList;
			}
			catch (ClientErrorException e) {
				_isErrorOccured = true;
				_errorMessage = e.Message;
			}
		}

		private async Task CreateRoomAsync() {
			_isErrorOccured = false;
			try {
				await _client.CreateRoomAsync(_roomGroupIndex, _roomName);
			}
			catch (ClientErrorException e) {
				_isErrorOccured = true;
				_errorMessage = e.Message;
			}
		}

		private async Task JoinRoomAsync() {
			_isErrorOccured = false;
			try {
				var room_id = _roomList.First(r => r.Name == _roomName).RoomId;
				_joinedRoomHost = await _client.JoinRoomAsync(_roomGroupIndex, room_id);
				_isJoinedRoom = true;
			}
			catch (ClientErrorException e) {
				_isErrorOccured = true;
				_errorMessage = e.Message;
			}
		}

		private async Task RemoveHostingRoomAsync() {
			_isErrorOccured = false;
			try {
				await _client.RemoveHostingRoomAsync();
			}
			catch (ClientErrorException e) {
				_isErrorOccured = true;
				_errorMessage = e.Message;
			}
		}

		private bool IsTaskRunning() {
			return _currentTask != null && !_currentTask.IsCompleted;
		}
	}
}