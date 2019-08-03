using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Linq;
using System.Threading;
using System.Threading.Tasks;

namespace PlanetaGameLabo.MatchMaker {
    internal sealed class Program {
        private static void Main(string[] args) {
            var parsed_result = CommandLine.Parser.Default.ParseArguments<Options>(args);

            if (parsed_result.Tag != CommandLine.ParserResultType.Parsed) {
                return;
            }

            var parsed = (CommandLine.Parsed<Options>) parsed_result;
            parsed.Value.Display();

            var client_list = new List<TestClient>();
            for (var i = 0; i < parsed.Value.ClientCount; ++i) {
                client_list.Add(new TestClient(parsed.Value.ServerAddress, parsed.Value.ServerPort));
            }

            Console.WriteLine("Start test.");

            var benchmark_results = new ConcurrentDictionary<string, ConcurrentQueue<long>>();
            var task_list = new List<Task>();
            foreach (var client in client_list) {
                Task task;
                switch (parsed.Value.Mode) {
                    case Mode.ConnectAndStay:
                        task = Task.Run(async () => await client.RunConnectAndStayTest(benchmark_results));
                        break;
                    case Mode.ConnectAndDisconnect:
                        task = Task.Run(async () => await client.RunConnectAndDisconnectTest(benchmark_results));
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

                    var count = pair.Value.Count;
                    var result_list = new List<long>();
                    for (var i = 0; i < count; ++i) {
                        if (!pair.Value.TryDequeue(out var result)) {
                            break;
                        }

                        result_list.Add(result);
                    }

                    line_list.Add($"{pair.Key}: {result_list.Average()}ms");
                }

                if (line_list.Count == 0) {
                    continue;
                }

                Console.WriteLine("--------Benchmark Results--------");
                line_list.ForEach(Console.WriteLine);
                Console.WriteLine("---------------------------------");
            }
        }
    }

    enum Mode {
        ConnectAndStay,
        ConnectAndDisconnect
    };

    class Options {
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