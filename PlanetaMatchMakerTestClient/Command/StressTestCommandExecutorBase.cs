using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Threading;
using System.Threading.Tasks;

namespace PlanetaGameLabo.MatchMaker
{
    internal abstract class StressTestCommandExecutorBase<TOptions> : CommandExecutorBase<TOptions>
        where TOptions : StressTestCommandOptions
    {
        protected StressTestCommandExecutorBase(StreamWriter outputStream) : base(outputStream)
        {
        }

        protected override async Task Execute(MatchMakerClient sharedClient, TOptions options,
            CancellationToken cancellationToken)
        {
            var testClientList = new List<MatchMakerClient>();
            try
            {
                // create clients for stress test
                for (var i = 0; i < options.ClientCount; ++i)
                {
                    testClientList.Add(new MatchMakerClient());
                }

                var benchmarkResults = new ConcurrentDictionary<string, ConcurrentQueue<(int, double)>>();

                // create tasks for stress test
                var taskList = testClientList
                    .Select(client => Task.Run(async () =>
                        await StressTest(client, benchmarkResults, options, cancellationToken)))
                    .ToList();

                // monitor stress test
                OutputStream.WriteLine("Start stress test.");
                while (taskList.Any(task => !task.IsCompleted))
                {
                    await Task.Delay(1000);

                    var lineList = new List<string>();
                    foreach (var pair in benchmarkResults)
                    {
                        if (pair.Value.IsEmpty)
                        {
                            continue;
                        }

                        var queueCount = pair.Value.Count;
                        var resultList = new List<double>();
                        var resultCount = 0;
                        for (var i = 0; i < queueCount; ++i)
                        {
                            if (!pair.Value.TryDequeue(out var result))
                            {
                                break;
                            }

                            var (count, averageTime) = result;
                            resultCount += count;
                            resultList.Add(averageTime);
                        }

                        lineList.Add(
                            $"{pair.Key}: response={resultList.Average():f03}ms, operation={resultCount}/s");
                    }

                    if (lineList.Count == 0)
                    {
                        continue;
                    }

                    OutputStream.WriteLine("--------Benchmark Results--------");
                    lineList.ForEach(Console.WriteLine);
                    OutputStream.WriteLine("---------------------------------");
                }

                OutputStream.WriteLine("Stress test is finished.");
            }
            finally
            {
                testClientList.ForEach(c => c.Dispose());
            }
        }

        public abstract Task StressTest(MatchMakerClient client,
            ConcurrentDictionary<string, ConcurrentQueue<(int, double)>> benchmarkResults,
            TOptions options, CancellationToken cancellationToken);

        protected readonly Stopwatch Stopwatch = new Stopwatch();
    }

    internal class StressTestCommandOptions
    {
        [CommandLine.Value(0, MetaName = "ServerAddress", Required = true, HelpText = "An address of the server.")]
        public string ServerAddress { get; set; }

        [CommandLine.Value(1, MetaName = "ServerPort", Required = true, HelpText = "A port of the server.")]
        public ushort ServerPort { get; set; }

        [CommandLine.Value(2, MetaName = "ClientCount", Required = true, HelpText = "The number of clients.")]
        public int ClientCount { get; set; }
    }
}