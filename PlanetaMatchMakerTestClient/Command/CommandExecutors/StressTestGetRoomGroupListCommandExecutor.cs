using System;
using System.Collections.Concurrent;
using System.IO;
using System.Threading;
using System.Threading.Tasks;

namespace PlanetaGameLabo.MatchMaker
{
    internal sealed class
        StressTestGetRoomListCommandExecutor : StressTestCommandExecutorBase<StressTestCommandOptions>
    {
        public StressTestGetRoomListCommandExecutor(StreamWriter outputStream) : base(outputStream)
        {
        }

        public override string Explanation => "Connect and get room repeatedly.";
        public override Command command => Command.StressTestGetRoomList;

        protected override async Task StressTest(MatchMakerClient client,
            ConcurrentDictionary<string, ConcurrentQueue<(int, double)>> benchmarkResults,
            StressTestCommandOptions options, CancellationToken cancellationToken)
        {
            var getRoomGroupListResponseBenchmarkResults =
                benchmarkResults.GetOrAdd("get_room_list", new ConcurrentQueue<(int, double)>());

            var playerName = Guid.NewGuid().ToString("N").Substring(0, 10);
            await client.ConnectAsync(options.ServerAddress, options.ServerPort, playerName);
            while (true)
            {
                Stopwatch.Restart();
                await client.GetRoomListAsync(0, 100, RoomDataSortKind.CreateDatetimeAscending);
                Stopwatch.Stop();
                getRoomGroupListResponseBenchmarkResults.Enqueue((1, Stopwatch.ElapsedMilliseconds));
                if (cancellationToken.IsCancellationRequested)
                {
                    return;
                }
            }
        }
    }
}