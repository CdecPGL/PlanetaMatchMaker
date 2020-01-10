using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Threading;
using System.Threading.Tasks;

namespace PlanetaGameLabo.MatchMaker
{
    internal abstract class TestCommandExecutorBase<TOptions> : CommandExecutorBase<TOptions>
        where TOptions : TestCommandOptions
    {
        protected class TestFailedException : Exception
        {
            public bool IsContinuable { get; }

            public TestFailedException(bool isContinuable, string message) : base(message)
            {
                IsContinuable = isContinuable;
            }
        }

        protected TestCommandExecutorBase(StreamWriter outputStream) : base(outputStream)
        {
        }

        protected override async Task Execute(MatchMakerClient sharedClient, TOptions options,
            CancellationToken cancellationToken)
        {
            try
            {
                sharedClient.Logger.Enabled = options.Verbose;
                var tests = new List<(string name, Func<MatchMakerClient, TOptions, CancellationToken, Task>)>();
                InitializeTest(tests);
                await ExecuteTests(tests, sharedClient, options, cancellationToken);
            }
            finally
            {
                FinalizeTest(sharedClient, options);
            }
        }

        protected abstract void InitializeTest(
            List<(string name, Func<MatchMakerClient, TOptions, CancellationToken, Task>)> tests);

        protected abstract void FinalizeTest(MatchMakerClient sharedClient, TOptions options);

        private struct TestResult
        {
            public readonly bool IsSucceeded;
            public readonly long TimeMilliSeconds;
            public readonly bool IsContinuable;

            public TestResult(bool isSucceeded, long timeMilliSeconds, bool isContinuable)
            {
                IsSucceeded = isSucceeded;
                TimeMilliSeconds = timeMilliSeconds;
                IsContinuable = isContinuable;
            }
        }

        private readonly Stopwatch stopwatch = new Stopwatch();

        private async Task ExecuteTests(
            IReadOnlyCollection<(string name, Func<MatchMakerClient, TOptions, CancellationToken, Task>)> tests,
            MatchMakerClient sharedClient, TOptions options, CancellationToken cancellationToken)
        {
            var testResults = new List<TestResult>();
            foreach (var (name, test) in tests)
            {
                var result = await ExecuteTest(name, test, sharedClient, options, cancellationToken);
                testResults.Add(result);
                if (!result.IsContinuable)
                {
                    OutputStream.WriteLine("Stop test because not continuable error is occured.");
                    break;
                }

                if (cancellationToken.IsCancellationRequested)
                {
                    OutputStream.WriteLine("Canceled by user.");
                    break;
                }
            }

            OutputStream.WriteLine("---Summary---");
            var totalExecuteTimeMilliSeconds = testResults.Sum(r => r.TimeMilliSeconds);
            foreach (var (result, (name, _)) in testResults.Zip(tests))
            {
                var statusText = result.IsSucceeded ? "Succeeded" : "Failed";
                OutputStream.WriteLine($"{name}: {statusText} ({result.TimeMilliSeconds} milli seconds)");
            }

            OutputStream.WriteLine($"Total time: {totalExecuteTimeMilliSeconds} milli seconds");

            var succeedCount = testResults.Count(r => r.IsSucceeded);
            var failedCount = testResults.Count(r => !r.IsSucceeded);
            var notExecutedCount = tests.Count - testResults.Count;
            OutputStream.WriteLine($"Succeed/Failed/NotExecuted: {succeedCount}/{failedCount}/{notExecutedCount}");
            OutputStream.WriteLine(failedCount + notExecutedCount == 0
                ? "All tests are succeeded."
                : "Some tests are failed.");
        }

        private async Task<TestResult> ExecuteTest(string name,
            Func<MatchMakerClient, TOptions, CancellationToken, Task> test, MatchMakerClient sharedClient,
            TOptions options, CancellationToken cancellationToken)
        {
            try
            {
                OutputStream.WriteLine($"---Start {name}---");
                stopwatch.Restart();
                await test(sharedClient, options, cancellationToken);
                stopwatch.Stop();
                OutputStream.WriteLine("Test is succeeded.");
                return new TestResult(true, stopwatch.ElapsedMilliseconds, true);
            }
            catch (TestFailedException e)
            {
                stopwatch.Stop();
                OutputStream.WriteLine($"Test is failed: {e.Message}");
                return new TestResult(false, stopwatch.ElapsedMilliseconds, e.IsContinuable);
            }
            finally
            {
                OutputStream.WriteLine($"Test time: {stopwatch.ElapsedMilliseconds} milli seconds.");
                OutputStream.WriteLine($"---Finish {name}---");
            }
        }
    }

    internal class TestCommandOptions
    {
        [CommandLine.Option('v', "Verbose", Default = false, Required = false,
            HelpText = "Output detailed information if true.")]
        public bool Verbose { get; set; }
    }
}