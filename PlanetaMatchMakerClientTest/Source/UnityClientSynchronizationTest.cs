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

        private static Dictionary<string, string> ReadSources(string root)
        {
            return Directory.GetFiles(root, "*.cs", SearchOption.AllDirectories)
                .ToDictionary(path => path.Substring(root.Length + 1).Replace(Path.DirectorySeparatorChar, '/'),
                    File.ReadAllText, StringComparer.Ordinal);
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
