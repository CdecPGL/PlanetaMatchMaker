using System;
using System.Collections.Generic;
using System.Threading.Tasks;

namespace PlanetaGameLabo.MatchMaker {
    internal sealed class Program {
        private static void Main(string[] args) {
            var result = CommandLine.Parser.Default.ParseArguments<Options>(args);

            if (result.Tag != CommandLine.ParserResultType.Parsed) {
                return;
            }

            var parsed = (CommandLine.Parsed<Options>) result;
            parsed.Value.Display();

            var client_list = new List<TestClient>();
            for (var i = 0; i < parsed.Value.ClientCount; ++i) {
                client_list.Add(new TestClient(parsed.Value.ServerAddress, parsed.Value.ServerPort));
            }

            var task_list = new List<Task>();
            foreach (var client in client_list) {
                Task task;
                switch (parsed.Value.Mode) {
                    case Mode.ConnectAndStay:
                        task = client.RunConnectAndStayTest();
                        break;
                    case Mode.ConnectAndDisconnect:
                        task = client.RunConnectAndDisconnectTest();
                        break;
                    default:
                        throw new ArgumentOutOfRangeException();
                }

                task_list.Add(task);
            }

            Task.WhenAll(task_list).Wait();
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