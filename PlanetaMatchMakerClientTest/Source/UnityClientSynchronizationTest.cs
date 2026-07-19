using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace PlanetaGameLabo.MatchMaker.Test
{
    [TestClass]
    public class UnityClientSynchronizationTest
    {
        [TestMethod]
        public void AllClientSourcesMatchUnityCopies()
        {
            var repositoryRoot = FindRepositoryRoot();
            var clientRoot = Path.Combine(repositoryRoot, "PlanetaMatchMakerClient", "Source");
            var unityRoot = Path.Combine(repositoryRoot, "PlanetaMatchMakerUnityClient", "Assets", "PlanetaGameLabo",
                "MatchMaker", "PlanetaMatchMakerClient");
            var clientSources = ReadSources(clientRoot);
            var unitySources = ReadSources(unityRoot);

            CollectionAssert.AreEqual(clientSources.Keys.OrderBy(path => path).ToArray(),
                unitySources.Keys.OrderBy(path => path).ToArray(),
                "The .NET and Unity clients do not contain the same C# source files.");
            foreach (var relativePath in clientSources.Keys)
            {
                Assert.AreEqual(clientSources[relativePath], unitySources[relativePath],
                    $"Unity client copy is out of sync: {relativePath}");
            }
        }

        [TestMethod]
        public void UnityWrapperPreservesAuthenticationErrorDetails()
        {
            var repositoryRoot = FindRepositoryRoot();
            var wrapperPath = Path.Combine(repositoryRoot, "PlanetaMatchMakerUnityClient", "Assets",
                "PlanetaGameLabo", "MatchMaker", "Runtime", "PlanetaMatchMakerClient.cs");
            var wrapperSource = File.ReadAllText(wrapperPath);

            StringAssert.Contains(wrapperSource, "case AuthenticationErrorException ae:");
            StringAssert.Contains(wrapperSource, "authenticationErrorCode = ae.AuthenticationErrorCode;");
            StringAssert.Contains(wrapperSource, "serverApiVersion = ae.ServerApiVersion;");
            StringAssert.Contains(wrapperSource, "serverGameVersion = ae.ServerGameVersion;");
            StringAssert.Contains(wrapperSource,
                "public readonly AuthenticationErrorCode? authenticationErrorCode;");
            StringAssert.Contains(wrapperSource, "public readonly ushort? serverApiVersion;");
            StringAssert.Contains(wrapperSource, "public readonly string serverGameVersion;");
        }

        [TestMethod]
        public void UnitySteamJoinPropagatesFailuresBeforeParsingPeerId()
        {
            var repositoryRoot = FindRepositoryRoot();
            var extensionPath = Path.Combine(repositoryRoot, "PlanetaMatchMakerUnityClient", "Assets",
                "PlanetaGameLabo", "MatchMaker", "Runtime", "Extensions", "Steam.cs");
            var extensionSource = File.ReadAllText(extensionPath);
            var callbackMethod = extensionSource.IndexOf("public static void JoinRoomWithSteam", StringComparison.Ordinal);
            var asyncMethod = extensionSource.IndexOf("JoinRoomWithSteamAsync", callbackMethod,
                StringComparison.Ordinal);

            Assert.IsTrue(callbackMethod >= 0);
            Assert.IsTrue(asyncMethod > callbackMethod);
            AssertFailureCheckPrecedesPeerIdParsing(extensionSource, callbackMethod, asyncMethod);
            AssertFailureCheckPrecedesPeerIdParsing(extensionSource, asyncMethod, extensionSource.Length);
            Assert.AreEqual(-1, extensionSource.IndexOf("ulong.Parse", StringComparison.Ordinal));
        }

        private static Dictionary<string, string> ReadSources(string root)
        {
            return Directory.GetFiles(root, "*.cs", SearchOption.AllDirectories)
                .ToDictionary(path => path.Substring(root.Length + 1).Replace(Path.DirectorySeparatorChar, '/'),
                    File.ReadAllText, StringComparer.Ordinal);
        }

        private static void AssertFailureCheckPrecedesPeerIdParsing(string source, int startIndex, int endIndex)
        {
            var failureCheck = source.IndexOf("if (!errorInfo)", startIndex, endIndex - startIndex,
                StringComparison.Ordinal);
            var parserCall = source.IndexOf("SteamP2pServicePeerIdParser.TryParse", startIndex,
                endIndex - startIndex, StringComparison.Ordinal);

            Assert.IsTrue(failureCheck >= startIndex, "The Unity Steam Join API must inspect ErrorInfo.");
            Assert.IsTrue(parserCall > failureCheck,
                "The Unity Steam Join API must propagate an error before parsing the peer ID.");
        }

        private static string FindRepositoryRoot()
        {
            for (var directory = new DirectoryInfo(AppContext.BaseDirectory);
                 directory != null;
                 directory = directory.Parent)
            {
                if (Directory.Exists(Path.Combine(directory.FullName, "PlanetaMatchMakerClient")) &&
                    Directory.Exists(Path.Combine(directory.FullName, "PlanetaMatchMakerUnityClient")))
                {
                    return directory.FullName;
                }
            }

            throw new DirectoryNotFoundException("PlanetaMatchMaker repository root was not found.");
        }
    }
}
