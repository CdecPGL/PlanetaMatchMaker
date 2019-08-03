using System.Threading.Tasks;

namespace PlanetaGameLabo.MatchMaker {
    class TestClient {
        public TestClient(string address, ushort port) {
            _client = new MatchMakerClient();
            _address = address;
            _port = port;
        }

        public async Task RunConnectAndStayTest() {
            try {
                await _client.ConnectAsync(_address, _port);
            }
            catch (ClientErrorException e) {
                System.Console.WriteLine($"Client error: {e.Message}");
            }
        }

        public async Task RunConnectAndDisconnectTest() {
            try {
                while (true) {
                    await _client.ConnectAsync(_address, _port);
                    _client.Close();
                }
            }
            catch (ClientErrorException e) {
                System.Console.WriteLine($"Client error: {e.Message}");
            }
        }

        private readonly MatchMakerClient _client;
        private readonly string _address;
        private readonly ushort _port;
    }
}