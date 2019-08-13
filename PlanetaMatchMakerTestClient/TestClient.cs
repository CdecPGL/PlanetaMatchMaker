using System;
using System.Collections.Concurrent;
using System.Diagnostics;
using System.Threading.Tasks;

namespace PlanetaGameLabo.MatchMaker {
    internal sealed class TestClient : IDisposable {
        public TestClient(string address, ushort port) {
            client = new MatchMakerClient();
            this.address = address;
            this.port = port;
        }

        public async Task
            RunConnectAndStayTest(ConcurrentDictionary<string, ConcurrentQueue<(int, double)>> benchmarkResults) {
            var connectionResponseBenchmarkResults =
                benchmarkResults.GetOrAdd("connection", new ConcurrentQueue<(int, double)>());
            try {
                stopwatch.Restart();
                await client.ConnectAsync(address, port);
                stopwatch.Stop();
                connectionResponseBenchmarkResults.Enqueue((1, stopwatch.ElapsedMilliseconds));
                await EternalDelay();
            }
            catch (ClientErrorException e) {
                Console.WriteLine($"Client error: {e.Message}");
            }
        }

        public async Task RunConnectAndDisconnectTest(
            ConcurrentDictionary<string, ConcurrentQueue<(int, double)>> benchmarkResults) {
            var connectionResponseBenchmarkResults =
                benchmarkResults.GetOrAdd("connection", new ConcurrentQueue<(int, double)>());
            try {
                while (true) {
                    stopwatch.Restart();
                    await client.ConnectAsync(address, port);
                    stopwatch.Stop();
                    connectionResponseBenchmarkResults.Enqueue((1, stopwatch.ElapsedMilliseconds));
                    client.Close();
                }
            }
            catch (ClientErrorException e) {
                Console.WriteLine($"Client error: {e.Message}");
            }
        }

        public async Task RunGetRoomGroupListTest(
            ConcurrentDictionary<string, ConcurrentQueue<(int, double)>> benchmarkResults) {
            var getRoomGroupListResponseBenchmarkResults =
                benchmarkResults.GetOrAdd("get_room_group_list", new ConcurrentQueue<(int, double)>());
            try {
                await client.ConnectAsync(address, port);
                while (true) {
                    stopwatch.Restart();
                    await client.GetRoomGroupListAsync();
                    stopwatch.Stop();
                    getRoomGroupListResponseBenchmarkResults.Enqueue((1, stopwatch.ElapsedMilliseconds));
                }
            }
            catch (ClientErrorException e) {
                Console.WriteLine($"Client error: {e.Message}");
            }
        }

        public void Dispose() {
            client?.Dispose();
        }

        private readonly MatchMakerClient client;
        private readonly string address;
        private readonly ushort port;
        private readonly Stopwatch stopwatch = new Stopwatch();

        private static async Task EternalDelay() {
            while (true) {
                await Task.Delay(1000);
            }
        }
    }
}