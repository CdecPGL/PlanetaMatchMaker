using System;
using System.IO;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace PlanetaGameLabo.MatchMaker.Test
{
    [TestClass]
    public class UnityClientSynchronizationTest
    {
        [DataTestMethod]
        [DataRow("Authentication/AuthenticationOptions.cs")]
        [DataRow("Client/ClientConstants.cs")]
        [DataRow("Client/MatchMakerClient.cs")]
        [DataRow("Message/MessageUtilities.cs")]
        [DataRow("Message/Messages.cs")]
        [DataRow("Room/RoomConstants.cs")]
        public void ClientSourceMatchesUnityCopy(string relativePath)
        {
            var repositoryRoot = FindRepositoryRoot();
            var clientPath = Path.Combine(repositoryRoot, "PlanetaMatchMakerClient", "Source", relativePath);
            var unityPath = Path.Combine(repositoryRoot, "PlanetaMatchMakerUnityClient", "Assets", "PlanetaGameLabo",
                "MatchMaker", "PlanetaMatchMakerClient", relativePath);

            Assert.AreEqual(File.ReadAllText(clientPath), File.ReadAllText(unityPath),
                $"Unity client copy is out of sync: {relativePath}");
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
