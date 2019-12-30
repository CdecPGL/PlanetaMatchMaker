using System.Collections.Concurrent;
using System.IO;
using System.Threading;
using System.Threading.Tasks;

namespace PlanetaGameLabo.MatchMaker
{
    internal sealed class
        StressTestGetRoomGroupListCommandExecutor : StressTestCommandExecutorBase<StressTestCommandOptions>
    {
        public StressTestGetRoomGroupListCommandExecutor(StreamWriter outputStream) : base(outputStream)
        {
        }

        public override string Explanation => "Connect indicated clients.";

        public override async Task StressTest(MatchMakerClient client,
            ConcurrentDictionary<string, ConcurrentQueue<(int, double)>> benchmarkResults,
            StressTestCommandOptions options, CancellationToken cancellationToken)
        {
            var getRoomGroupListResponseBenchmarkResults =
                benchmarkResults.GetOrAdd("get_room_group_list", new ConcurrentQueue<(int, double)>());
            try
            {
                await client.ConnectAsync(options.ServerAddress, options.ServerPort);
                while (true)
                {
                    Stopwatch.Restart();
                    await client.GetRoomGroupListAsync();
                    Stopwatch.Stop();
                    getRoomGroupListResponseBenchmarkResults.Enqueue((1, Stopwatch.ElapsedMilliseconds));
                    if (cancellationToken.IsCancellationRequested)
                    {
                        return;
                    }
                }
            }
            catch (ClientErrorException e)
            {
                OutputStream.WriteLine($"Client error: {e.Message}");
            }
        }
    }
}