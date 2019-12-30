using System;
using System.Threading;
using System.Threading.Tasks;

namespace PlanetaGameLabo.MatchMaker
{
    internal interface ICommandExecutor
    {
        string Explanation { get; }
        Command command { get; }
        Task Execute(MatchMakerClient sharedClient, string[] args, CancellationToken cancellationToken);
    }

    internal class CommandExecutionErrorException : Exception
    {
        public CommandExecutionErrorException(string message) : base(message) { }
    }
}