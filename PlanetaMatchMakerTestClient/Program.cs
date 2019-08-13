using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Linq;
using System.Threading;
using System.Threading.Tasks;

namespace PlanetaGameLabo.MatchMaker {
    internal static class Program {
        private static void Main(string[] args) {
            var parsedResult = CommandLine.Parser.Default.ParseArguments<Options>(args);

            if (parsedResult.Tag != CommandLine.ParserResultType.Parsed) {
                return;
            }

            var parsed = (CommandLine.Parsed<Options>) parsedResult;
            parsed.Value.Display();

            Console.CancelKeyPress += (sender, eargs) => DisposeAllTestClients();
            try {
                for (var i = 0; i < parsed.Value.ClientCount; ++i) {
                    TestClientList.Add(new TestClient(parsed.Value.ServerAddress, parsed.Value.ServerPort));
                }

                Console.WriteLine("Start test.");

                var benchmarkResults = new ConcurrentDictionary<string, ConcurrentQueue<(int, double)>>();
                var taskList = new List<Task>();
                foreach (var client in TestClientList) {
                    Task task;
                    switch (parsed.Value.Mode) {
                        case Mode.ConnectAndStay:
                            task = Task.Run(async () => await client.RunConnectAndStayTest(benchmarkResults));
                            break;
                        case Mode.ConnectAndDisconnect:
                            task = Task.Run(async () => await client.RunConnectAndDisconnectTest(benchmarkResults));
                            break;
                        case Mode.GetRoomGroupList:
                            task = Task.Run(async () => await client.RunGetRoomGroupListTest(benchmarkResults));
                            break;
                        default:
                            throw new ArgumentOutOfRangeException();
                    }

                    taskList.Add(task);
                }

                while (taskList.Any(task => !task.IsCompleted)) {
                    Thread.Sleep(1000);
                    var lineList = new List<string>();
                    foreach (var pair in benchmarkResults) {
                        if (pair.Value.IsEmpty) {
                            continue;
                        }

                        var queueCount = pair.Value.Count;
                        var resultList = new List<double>();
                        var resultCount = 0;
                        for (var i = 0; i < queueCount; ++i) {
                            if (!pair.Value.TryDequeue(out var result)) {
                                break;
                            }

                            var (count, averageTime) = result;
                            resultCount += count;
                            resultList.Add(averageTime);
                        }

                        lineList.Add(
                            $"{pair.Key}: response={resultList.Average():f03}ms, operation={resultCount}/s");
                    }

                    if (lineList.Count == 0) {
                        continue;
                    }

                    Console.WriteLine("--------Benchmark Results--------");
                    lineList.ForEach(Console.WriteLine);
                    Console.WriteLine("---------------------------------");
                }
            }
            finally {
                DisposeAllTestClients();
            }
        }

        private static readonly List<TestClient> TestClientList = new List<TestClient>();

        private static void DisposeAllTestClients() {
            TestClientList.ForEach(client => client.Dispose());
        }
    }

    internal enum Mode {
        ConnectAndStay,
        ConnectAndDisconnect,
        GetRoomGroupList
    };

    internal sealed class Options {
        [CommandLine.Option('a', "server_address", Required = true, HelpText = "An address of the server.")]
        public string ServerAddress { get; set; }

        [CommandLine.Option('p', "server_port", Required = true, HelpText = "A port of the server.")]
        public ushort ServerPort { get; set; }

        [CommandLine.Option('m', "mode", Required = true, HelpText = "A mode of the test client.")]
        public Mode Mode { get; set; }

        [CommandLine.Option('c', "client_count", Required = true, HelpText = "The number of clients.")]
        public int ClientCount { get; set; }

        public void Display() {
            Console.WriteLine("========Options========");
            Console.WriteLine($"Server Address: {ServerAddress}");
            Console.WriteLine($"Server Port: {ServerPort}");
            Console.WriteLine($"Mode: {Mode}");
            Console.WriteLine($"ClientCount: {ClientCount}");
            Console.WriteLine("=======================");
        }
    }
}