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
            var command = options.Command;
            var commandOptions = options.CommandOptions;

            using (var client = new MatchMakerClient())
            {
                Console.CancelKeyPress += OnKeyBoardInterrupted;

                while (!isFinishProgram)
                {
                    if (command == null)
                    {
                        Console.WriteLine("Input command.");
                        commandProcessor.DisplayCommandList();
                        var input = Console.ReadLine();
                        if (input == null)
                        {
                            continue;
                        }

                        var items = input.Split(' ');
                        if (!CommandProcessor.TryParseCommand(input, out var c))
                        {
                            continue;
                        }

                        command = c;
                        commandOptions = items.Skip(1);
                    }
                    else
                    {
                        isCommandExecuting = true;
                        var task = commandProcessor.ProcessCommand(command.Value, client, commandOptions.ToArray(),
                            cancellationTokenSource.Token);
                        task.Wait();
                        isCommandExecuting = false;
                        command = null;
                    }
                }
            }
        }

        private static bool isCommandExecuting;
        private static bool isFinishProgram;
        private static readonly CancellationTokenSource cancellationTokenSource = new CancellationTokenSource();

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
        [CommandLine.Value(0, MetaName = "Command", Default = null, Required = false, HelpText = "Command.")]
        public Command? Command { get; set; }

        [CommandLine.Value(1, MetaName = "CommandOptions", Default = null, Required = false,
            HelpText = "Command options.")]
        public IEnumerable<string> CommandOptions { get; set; }
    }
}