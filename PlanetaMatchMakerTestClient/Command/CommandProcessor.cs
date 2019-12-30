using System.Collections.Generic;
using System.IO;
using System.Threading;
using System.Threading.Tasks;

namespace PlanetaGameLabo.MatchMaker
{
    internal sealed class CommandProcessor
    {
        public CommandProcessor(StreamWriter outputStream, Dictionary<Command, ICommandExecutor> commandExecutorMap)
        {
            this.outputStream = outputStream;
            this.commandExecutorMap = commandExecutorMap;
        }

        public void DisplayCommandList()
        {
            foreach (var commandExecutor in commandExecutorMap)
            {
                outputStream.WriteLine($"{commandExecutor.Key}: {commandExecutor.Value.Explanation}");
            }
        }

        public async Task ProcessCommand(Command command, MatchMakerClient client, string[] args, CancellationToken cancellationToken)
        {
            await commandExecutorMap[command].Execute(client, args, cancellationToken);
        }

        private readonly Dictionary<Command, ICommandExecutor> commandExecutorMap;
        private readonly StreamWriter outputStream;
    }
}