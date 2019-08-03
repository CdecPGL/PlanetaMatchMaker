using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Linq;
using System.Threading;
using System.Threading.Tasks;

namespace PlanetaGameLabo.MatchMaker {
    internal static class Program {
        private static void Main(string[] args) {
            var parsed_result = CommandLine.Parser.Default.ParseArguments<Options>(args);

            if (parsed_result.Tag != CommandLine.ParserResultType.Parsed) {
                return;
            }

            var parsed = (CommandLine.Parsed<Options>) parsed_result;
            parsed.Value.Display();

            Console.CancelKeyPress += (sender, eargs) => DisposeAllTestClients();
            try {
                for (var i = 0; i < parsed.Value.clientCount; ++i) {
                    _testClientList.Add(new TestClient(parsed.Value.serverAddress, parsed.Value.serverPort));
                }

                Console.WriteLine("Start test.");

                var benchmark_results = new ConcurrentDictionary<string, ConcurrentQueue<(int, double)>>();
                var task_list = new List<Task>();
                foreach (var client in _testClientList) {
                    Task task;
                    switch (parsed.Value.mode) {
                        case Mode.ConnectAndStay:
                            task = Task.Run(async () => await client.RunConnectAndStayTest(benchmark_results));
                            break;
                        case Mode.ConnectAndDisconnect:
                            task = Task.Run(async () => await client.RunConnectAndDisconnectTest(benchmark_results));
                            break;
                        case Mode.GetRoomGroupList:
                            task = Task.Run(async () => await client.RunGetRoomGroupListTest(benchmark_results));
                            break;
                        default:
                            throw new ArgumentOutOfRangeException();
                    }

                    task_list.Add(task);
                }

                while (task_list.Any(task => !task.IsCompleted)) {
                    Thread.Sleep(1000);
                    var line_list = new List<string>();
                    foreach (var pair in benchmark_results) {
                        if (pair.Value.IsEmpty) {
                            continue;
                        }

                        var queue_count = pair.Value.Count;
                        var result_list = new List<double>();
                        var result_count = 0;
                        for (var i = 0; i < queue_count; ++i) {
                            if (!pair.Value.TryDequeue(out var result)) {
                                break;
                            }

                            var (count, average_time) = result;
                            result_count += count;
                            result_list.Add(average_time);
                        }

                        line_list.Add(
                            $"{pair.Key}: response={result_list.Average():f03}ms, operation={result_count}/s");
                    }

                    if (line_list.Count == 0) {
                        continue;
                    }

                    Console.WriteLine("--------Benchmark Results--------");
                    line_list.ForEach(Console.WriteLine);
                    Console.WriteLine("---------------------------------");
                }
            }
            finally {
                DisposeAllTestClients();
            }
        }

        private static readonly List<TestClient> _testClientList = new List<TestClient>();

        private static void DisposeAllTestClients() {
            _testClientList.ForEach(client => client.Dispose());
        }
    }

    internal enum Mode {
        ConnectAndStay,
        ConnectAndDisconnect,
        GetRoomGroupList
    };

    internal sealed class Options {
        [CommandLine.Option('a', "server_address", Required = true, HelpText = "An address of the server.")]
        public string serverAddress { get; set; }

        [CommandLine.Option('p', "server_port", Required = true, HelpText = "A port of the server.")]
        public ushort serverPort { get; set; }

        [CommandLine.Option('m', "mode", Required = true, HelpText = "A mode of the test client.")]
        public Mode mode { get; set; }

        [CommandLine.Option('c', "client_count", Required = true, HelpText = "The number of clients.")]
        public int clientCount { get; set; }

        public void Display() {
            Console.WriteLine("========Options========");
            Console.WriteLine($"Server Address: {serverAddress}");
            Console.WriteLine($"Server Port: {serverPort}");
            Console.WriteLine($"Mode: {mode}");
            Console.WriteLine($"ClientCount: {clientCount}");
            Console.WriteLine("=======================");
        }
    }
}