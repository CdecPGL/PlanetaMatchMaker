using System.IO;
using System.Threading;
using System.Threading.Tasks;

namespace PlanetaGameLabo.MatchMaker
{
    internal class ListRoomCommandExecutor : StandardCommandExecutorBase<ListRoomCommandOptions>
    {
        public override string Explanation => "List rooms whose condition matches options.";
        public override Command command => Command.ListRoom;

        public ListRoomCommandExecutor(StreamWriter outputStream) : base(outputStream)
        {
        }

        protected override async Task ExecuteStandardCommand(MatchMakerClient sharedClient,
            ListRoomCommandOptions options,
            CancellationToken cancellationToken)
        {
            var (totalRoomCount, matchedRoomCount, results) = await sharedClient.GetRoomListAsync(
                options.StartIndex, options.Count, options.SortKind, options.SearchTargetFlag, options.SearchName,
                options.SearchTag);

            OutputStream.WriteLine($"{matchedRoomCount} rooms are matched in {totalRoomCount} rooms.");
            OutputStream.WriteLine($"{results.Length} rooms are replied from index {options.StartIndex}.");
            foreach (var result in results)
            {
                OutputStream.WriteLine(
                    $"    Name: {result.HostPlayerFullName}, ID: {result.RoomId}, Create: {result.CreateDatetime}, PlayerCount: {result.CurrentPlayerCount}/{result.MaxPlayerCount}, SettingFlags: {result.SettingFlags}, ConnectionEstablishMode: {result.ConnectionEstablishMode}");
            }
        }
    }

    internal class ListRoomCommandOptions : StandardCommandOptions
    {
        [CommandLine.Value(1, MetaName = "start_index", Required = true,
            HelpText = "An start index of result.")]
        public byte StartIndex { get; set; }

        [CommandLine.Value(2, MetaName = "count", Required = true,
            HelpText = "Count of result.")]
        public byte Count { get; set; }

        [CommandLine.Value(3, MetaName = "sort_kind", Required = true,
            HelpText = "Sort method of result.")]
        public RoomDataSortKind SortKind { get; set; }

        [CommandLine.Option('f', "search_target_flag", Default = RoomSearchTargetFlag.All, Required = false,
            HelpText = "Flag which indicates condition of search.")]
        public RoomSearchTargetFlag SearchTargetFlag { get; set; }

        [CommandLine.Option('n', "search_name", Default = "", Required = false,
            HelpText = "A name to search room. Empty string means search all.")]
        public string SearchName { get; set; }

        [CommandLine.Option('t', "search_tag", Default = (ushort)0, Required = false,
            HelpText = "A tag to search room. 0 means search all.")]
        public ushort SearchTag { get; set; }
    }
}