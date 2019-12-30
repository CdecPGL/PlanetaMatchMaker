using System.IO;
using System.Threading;
using System.Threading.Tasks;
using CommandLine.Text;

namespace PlanetaGameLabo.MatchMaker
{
    internal abstract class CommandExecutorBase<TOptions> : ICommandExecutor
    {
        public CommandExecutorBase(StreamWriter outputStream)
        {
            OutputStream = outputStream;
        }

        public abstract string Explanation { get; }

        public async Task Execute(MatchMakerClient sharedClient, string[] args, CancellationToken cancellationToken)
        {
            var parsedResult = CommandLine.Parser.Default.ParseArguments<TOptions>(args);
            if (parsedResult.Tag != CommandLine.ParserResultType.Parsed)
            {
                var helpText = HelpText.AutoBuild(parsedResult);
                OutputStream.WriteLine("Failed to parse options.");
                OutputStream.WriteLine(helpText);
                return;
            }

            var options = ((CommandLine.Parsed<TOptions>)parsedResult).Value;
            await Execute(sharedClient, options, cancellationToken);
        }

        protected abstract Task Execute(MatchMakerClient sharedClient, TOptions options,
            CancellationToken cancellationToken);

        protected readonly StreamWriter OutputStream;
    }
}