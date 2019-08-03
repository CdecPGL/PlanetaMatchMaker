using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace PlanetaGameLabo.MatchMaker {
    class Program {
        static void Main(string[] args) {
            var result = CommandLine.Parser.Default.ParseArguments<Options>(args);

            if (result.Tag != CommandLine.ParserResultType.Parsed) {
                // パースに失敗していれば、NotParsed<T>型に変換できるはずだ！ (使わんけど……)
                var notParsed = (CommandLine.NotParsed<Options>) result;

                Console.WriteLine("コマンドライン引数を間違えてないかぃ？");
                return;
            }

            // パースに成功していれば、Parsed<T>型に変換できるはずだ！
            var parsed = (CommandLine.Parsed<Options>) result;

            Console.WriteLine($"Mode: {parsed.Value.Mode}");
            Console.WriteLine($"ClientCount: {parsed.Value.ClientCount}");


            var client_list = new List<TestClient>();
            for (var i = 0; i < parsed.Value.ClientCount; ++i) {
                client_list.Add(new TestClient("127.0.0.1", 7777));
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
        [CommandLine.Option('m', "mode", Required = true, HelpText = "テストのモード")]
        public Mode Mode { get; set; }

        [CommandLine.Option('c', "client_count", Required = true, HelpText = "作成するクライアントの数")]
        public int ClientCount { get; set; }
    }
}