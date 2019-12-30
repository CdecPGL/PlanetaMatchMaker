using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Threading;
using System.Threading.Tasks;

namespace PlanetaGameLabo.MatchMaker
{
    internal sealed class CommandProcessor
    {
        public CommandProcessor(StreamWriter outputStream, IEnumerable<ICommandExecutor> commandExecutors)
        {
            this.outputStream = outputStream;
            commandExecutorMap = commandExecutors.ToDictionary(e => e.command, e => e);
        }

        public void DisplayCommandList()
        {
            foreach (var commandExecutor in commandExecutorMap)
            {
                outputStream.WriteLine($"{commandExecutor.Key}: {commandExecutor.Value.Explanation}");
            }
        }

        public async Task ProcessCommand(Command command, MatchMakerClient client, string[] args,
            CancellationToken cancellationToken)
        {
            outputStream.WriteLine($"=====Start {command} Command=====");
            try
            {
                await commandExecutorMap[command].Execute(client, args, cancellationToken);
            }
            catch (ClientErrorException e)
            {
                outputStream.WriteLine($"Error occurred in the client: {e.Message}");
            }
            catch (CommandExecutionErrorException e)
            {
                outputStream.WriteLine($"Failed to execute command: {e.Message}");
            }
            catch (Exception e)
            {
                outputStream.WriteLine($"Unknown error: {e}");
            }

            outputStream.WriteLine($"=====Finish {command} Command====");
        }

        private readonly Dictionary<Command, ICommandExecutor> commandExecutorMap;
        private readonly StreamWriter outputStream;
    }
}