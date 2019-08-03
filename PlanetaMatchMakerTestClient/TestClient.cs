using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace PlanetaGameLabo.MatchMaker
{
    class TestClient {
        public TestClient(string address, ushort port) {
            _client = new MatchMakerClient();
            _address = address;
            _port = port;
        }

        public async Task RunConnectAndStayTest() {
            await _client.ConnectAsync(_address, _port);
        }

        public async Task RunConnectAndDisconnectTest() {
            while (true) {
                await _client.ConnectAsync(_address, _port);
                _client.Close();
            }
        }

        private MatchMakerClient _client;
        private readonly string _address;
        private readonly ushort _port;
    }
}
