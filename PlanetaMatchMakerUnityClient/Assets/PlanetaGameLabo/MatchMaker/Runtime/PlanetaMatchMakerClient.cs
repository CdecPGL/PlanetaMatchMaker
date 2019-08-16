using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

namespace PlanetaGameLabo.MatchMaker {
	public sealed class PlanetaMatchMakerClient : MonoBehaviour {
		/// <summary>
		/// true if connected to the server.
		/// </summary>
		public bool connected => _client.Connected;

		/// <summary>
		/// true if this client hosting a room.
		/// </summary>
		public bool isHostingRoom => _client.IsHostingRoom;

		private MatchMakerClient _client;
		
		private void Update(){
			
		}
	}
}