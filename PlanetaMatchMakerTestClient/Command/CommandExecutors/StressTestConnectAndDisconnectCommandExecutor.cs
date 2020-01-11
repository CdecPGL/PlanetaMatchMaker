using System;
using System.Collections.Concurrent;
using System.IO;
using System.Threading;
using System.Threading.Tasks;

namespace PlanetaGameLabo.MatchMaker
{
    internal sealed class
        StressTestConnectAndDisconnectCommandExecutor : StressTestCommandExecutorBase<StressTestCommandOptions>
    {
        public StressTestConnectAndDisconnectCommandExecutor(StreamWriter outputStream) : base(outputStream)
        {
        }

        public override string Explanation => "Connect and disconnect repeatedly.";
        public override Command command => Command.StressTestConnectAndDisconnect;

        protected override async Task StressTest(MatchMakerClient client,
            ConcurrentDictionary<string, ConcurrentQueue<(int, double)>> benchmarkResults,
            StressTestCommandOptions options, CancellationToken cancellationToken)
        {
            var connectionResponseBenchmarkResults =
                benchmarkResults.GetOrAdd("connection", new ConcurrentQueue<(int, double)>());
            while (true)
            {
                var playerName = Guid.NewGuid().ToString("N").Substring(0, 10);
                Stopwatch.Restart();
                await client.ConnectAsync(options.ServerAddress, options.ServerPort, playerName);
                Stopwatch.Stop();
                connectionResponseBenchmarkResults.Enqueue((1, Stopwatch.ElapsedMilliseconds));
                client.Close();
                if (cancellationToken.IsCancellationRequested)
                {
                    return;
                }
            }
        }
    }
}