using System.Collections.Concurrent;
using System.Diagnostics;
using System.Threading;
using System.Threading.Tasks;

namespace PlanetaGameLabo.MatchMaker {
    class TestClient {
        public TestClient(string address, ushort port) {
            _client = new MatchMakerClient();
            _address = address;
            _port = port;
        }

        public async Task RunConnectAndStayTest(ConcurrentDictionary<string, ConcurrentQueue<long>> benchmark_results) {
            //System.Console.WriteLine(
            //    $"Start ConnectAndStay test in the thread {Thread.CurrentThread.ManagedThreadId}.");
            var connection_response_benchmark_results =
                benchmark_results.GetOrAdd("connection_response_time", new ConcurrentQueue<long>());
            try {
                _stopwatch.Restart();
                await _client.ConnectAsync(_address, _port);
                _stopwatch.Stop();
                connection_response_benchmark_results.Enqueue(_stopwatch.ElapsedMilliseconds);
                await EternalDelay();
            }
            catch (ClientErrorException e) {
                System.Console.WriteLine($"Client error: {e.Message}");
            }
        }

        public async Task RunConnectAndDisconnectTest(
            ConcurrentDictionary<string, ConcurrentQueue<long>> benchmark_results) {
            //System.Console.WriteLine(
            //    $"Start ConnectAndDisconnect test in the thread {Thread.CurrentThread.ManagedThreadId}.");
            var connection_response_benchmark_results =
                benchmark_results.GetOrAdd("connection_response_time", new ConcurrentQueue<long>());
            try {
                while (true) {
                    _stopwatch.Restart();
                    await _client.ConnectAsync(_address, _port);
                    _stopwatch.Stop();
                    connection_response_benchmark_results.Enqueue(_stopwatch.ElapsedMilliseconds);
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
        private readonly Stopwatch _stopwatch = new System.Diagnostics.Stopwatch();

        private static async Task EternalDelay() {
            while (true) {
                await Task.Delay(1000);
            }
        }
    }
}