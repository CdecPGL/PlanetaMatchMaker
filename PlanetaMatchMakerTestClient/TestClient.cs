using System;
using System.Collections.Concurrent;
using System.Diagnostics;
using System.Threading.Tasks;

namespace PlanetaGameLabo.MatchMaker {
    internal sealed class TestClient : IDisposable {
        public TestClient(string address, ushort port) {
            _client = new MatchMakerClient();
            _address = address;
            _port = port;
        }

        public async Task
            RunConnectAndStayTest(ConcurrentDictionary<string, ConcurrentQueue<(int, double)>> benchmark_results) {
            var connection_response_benchmark_results =
                benchmark_results.GetOrAdd("connection", new ConcurrentQueue<(int, double)>());
            try {
                _stopwatch.Restart();
                await _client.ConnectAsync(_address, _port);
                _stopwatch.Stop();
                connection_response_benchmark_results.Enqueue((1, _stopwatch.ElapsedMilliseconds));
                await EternalDelay();
            }
            catch (ClientErrorException e) {
                Console.WriteLine($"Client error: {e.Message}");
            }
        }

        public async Task RunConnectAndDisconnectTest(
            ConcurrentDictionary<string, ConcurrentQueue<(int, double)>> benchmark_results) {
            var connection_response_benchmark_results =
                benchmark_results.GetOrAdd("connection", new ConcurrentQueue<(int, double)>());
            try {
                while (true) {
                    _stopwatch.Restart();
                    await _client.ConnectAsync(_address, _port);
                    _stopwatch.Stop();
                    connection_response_benchmark_results.Enqueue((1, _stopwatch.ElapsedMilliseconds));
                    _client.Close();
                }
            }
            catch (ClientErrorException e) {
                Console.WriteLine($"Client error: {e.Message}");
            }
        }

        public async Task RunGetRoomGroupListTest(
            ConcurrentDictionary<string, ConcurrentQueue<(int, double)>> benchmark_results) {
            var get_room_group_list_response_benchmark_results =
                benchmark_results.GetOrAdd("get_room_group_list", new ConcurrentQueue<(int, double)>());
            try {
                await _client.ConnectAsync(_address, _port);
                while (true) {
                    _stopwatch.Restart();
                    await _client.GetRoomGroupListAsync();
                    _stopwatch.Stop();
                    get_room_group_list_response_benchmark_results.Enqueue((1, _stopwatch.ElapsedMilliseconds));
                }
            }
            catch (ClientErrorException e) {
                Console.WriteLine($"Client error: {e.Message}");
            }
        }

        public void Dispose() {
            _client?.Dispose();
        }

        private readonly MatchMakerClient _client;
        private readonly string _address;
        private readonly ushort _port;
        private readonly Stopwatch _stopwatch = new Stopwatch();

        private static async Task EternalDelay() {
            while (true) {
                await Task.Delay(1000);
            }
        }
    }
}