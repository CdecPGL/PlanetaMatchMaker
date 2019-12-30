using System.Collections.Concurrent;
using System.IO;
using System.Threading;
using System.Threading.Tasks;

namespace PlanetaGameLabo.MatchMaker
{
    internal sealed class
        StressTestConnectAndStayCommandExecutor : StressTestCommandExecutorBase<StressTestCommandOptions>
    {
        public StressTestConnectAndStayCommandExecutor(StreamWriter outputStream) : base(outputStream)
        {
        }

        public override string Explanation => "Connect indicated clients.";
        public override Command command => Command.StressTestConnectAndStay;

        protected override async Task StressTest(MatchMakerClient client,
            ConcurrentDictionary<string, ConcurrentQueue<(int, double)>> benchmarkResults,
            StressTestCommandOptions options, CancellationToken cancellationToken)
        {
            var connectionResponseBenchmarkResults =
                benchmarkResults.GetOrAdd("connection", new ConcurrentQueue<(int, double)>());
            Stopwatch.Restart();
            await client.ConnectAsync(options.ServerAddress, options.ServerPort);
            Stopwatch.Stop();
            connectionResponseBenchmarkResults.Enqueue((1, Stopwatch.ElapsedMilliseconds));
        }
    }
}