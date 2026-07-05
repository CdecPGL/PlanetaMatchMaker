using CommandLine;

namespace PlanetaGameLabo.MatchMaker
{
    internal static class CommandLineParserFactory
    {
        public static Parser Create()
        {
            return new Parser(settings => { settings.CaseInsensitiveEnumValues = true; });
        }
    }
}
