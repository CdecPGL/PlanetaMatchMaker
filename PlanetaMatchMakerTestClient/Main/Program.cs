using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading;
using CommandLine.Text;

namespace PlanetaGameLabo.MatchMaker
{
    internal static class Program
    {
        private static void Main(string[] args)
        {
            var parsedResult = CommandLine.Parser.Default.ParseArguments<Options>(args);

            if (parsedResult.Tag != CommandLine.ParserResultType.Parsed)
            {
                var helpText = HelpText.AutoBuild(parsedResult);
                Console.WriteLine("Failed to parse options.");
                Console.WriteLine(helpText);
                return;
            }

            var options = ((CommandLine.Parsed<Options>)parsedResult).Value;
            var commandProcessor = CommandProcessorFactory.Create();

            try
            {
                using var client = new MatchMakerClient();
                Console.CancelKeyPress += OnKeyBoardInterrupted;

                if (!string.IsNullOrEmpty(options.CommandAndOptions))
                {
                    var items = options.CommandAndOptions.Split(' ');
                    if (!CommandProcessor.TryParseCommand(items[0], out var command))
                    {
                        Console.WriteLine("Command is invalid.");
                    }
                    else
                    {
                        var commandOptions = items.Skip(1);
                        ExecuteCommand(commandProcessor, command, client, commandOptions);
                    }
                }
                else
                {
                    while (!isFinishProgram)
                    {
                        Console.WriteLine("Input command.");
                        commandProcessor.DisplayCommandList();
                        var input = Console.ReadLine();
                        if (input == null)
                        {
                            Console.WriteLine("Command is invalid.");
                            continue;
                        }

                        var items = input.Split(' ');
                        if (!CommandProcessor.TryParseCommand(items[0], out var command))
                        {
                            Console.WriteLine("Command is invalid.");
                            continue;
                        }

                        var commandOptions = items.Skip(1);
                        ExecuteCommand(commandProcessor, command, client, commandOptions);
                    }
                }
            }
            finally
            {
                NatPortMappingCreator.ReleaseCreatedPortMappings();
            }
        }

        private static bool isCommandExecuting;
        private static bool isFinishProgram;
        private static readonly CancellationTokenSource cancellationTokenSource = new CancellationTokenSource();

        private static void ExecuteCommand(CommandProcessor commandProcessor, Command command, MatchMakerClient client,
            IEnumerable<string> commandOptions)
        {
            isCommandExecuting = true;
            var task = commandProcessor.ProcessCommand(command, client, commandOptions.ToArray(),
                cancellationTokenSource.Token);
            task.Wait();
            isCommandExecuting = false;
        }

        private static void OnKeyBoardInterrupted(object sender, ConsoleCancelEventArgs eargs)
        {
            cancellationTokenSource.Cancel();
            if (isCommandExecuting)
            {
                eargs.Cancel = true;
            }
            else
            {
                isFinishProgram = true;
            }
        }
    }

    internal sealed class Options
    {
        [CommandLine.Value(0, MetaName = "command_and_options", Default = null, Required = false,
            HelpText = "Command.")]
        public string CommandAndOptions { get; set; }
    }
}