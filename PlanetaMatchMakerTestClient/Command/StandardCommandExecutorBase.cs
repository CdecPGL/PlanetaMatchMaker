using System.Diagnostics;
using System.IO;
using System.Threading;
using System.Threading.Tasks;

namespace PlanetaGameLabo.MatchMaker
{
    internal abstract class StandardCommandExecutorBase<TOptions> : CommandExecutorBase<TOptions>
        where TOptions : StandardCommandOptions
    {
        protected StandardCommandExecutorBase(StreamWriter outputStream) : base(outputStream)
        {
        }

        protected override async Task Execute(MatchMakerClient sharedClient, TOptions options,
            CancellationToken cancellationToken)
        {
            sharedClient.Logger.Enabled = options.Verbose;
            stopwatch.Restart();
            await ExecuteStandardCommand(sharedClient, options, cancellationToken);
            stopwatch.Stop();
            OutputStream.WriteLine($"<<{stopwatch.ElapsedMilliseconds} milli seconds>>");
        }

        protected abstract Task ExecuteStandardCommand(MatchMakerClient sharedClient, TOptions options,
            CancellationToken cancellationToken);

        private readonly Stopwatch stopwatch = new Stopwatch();
    }

    internal class StandardCommandOptions
    {
        [CommandLine.Option('v', "Verbose", Default = false, Required = false,
            HelpText = "Output detailed information if true.")]
        public bool Verbose { get; set; }
    }
}