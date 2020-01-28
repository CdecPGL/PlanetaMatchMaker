using System.Text;
using System.Text.RegularExpressions;

namespace PlanetaGameLabo.MatchMaker
{
    public static class Validator
    {
        /// <summary>
        /// Validate server address.
        /// IPv4, IPv6 or URL is valid.
        /// </summary>
        /// <param name="address"></param>
        /// <returns></returns>
        public static bool ValidateServerAddress(string address)
        {
            if (address == null)
            {
                return false;
            }

            // IPv4
            if (Regex.IsMatch(address, ipv4Regex))
            {
                return true;
            }

            // IPv6
            if (Regex.IsMatch(address, ipv6Regex))
            {
                return true;
            }

            // URL
            return Regex.IsMatch(address, urlRegex);
        }

        /// <summary>
        /// Validate server port.
        /// Port which is not 0 is valid.
        /// </summary>
        /// <param name="port"></param>
        /// <returns></returns>
        public static bool ValidateServerPort(ushort port)
        {
            return port != 0;
        }

        /// <summary>
        /// Validate player name.
        /// Not null string whose length is not more than limit is valid.
        /// </summary>
        /// <param name="name"></param>
        /// <returns></returns>
        public static bool ValidatePlayerName(string name)
        {
            return !string.IsNullOrWhiteSpace(name) &&
                   Encoding.UTF8.GetByteCount(name) <= ClientConstants.PlayerNameLength;
        }

        /// <summary>
        /// Validate a game hot port.
        /// dynamic/private port is valid.
        /// </summary>
        /// <param name="port"></param>
        /// <returns></returns>
        public static bool ValidateGameHostPort(ushort port)
        {
            return 49152 <= port && port <= 65535;
        }

        /// <summary>
        /// Validate a room password.
        /// Not null string whose length is not more than limit is valid.
        /// </summary>
        /// <param name="password"></param>
        /// <returns></returns>
        public static bool ValidateRoomPassword(string password)
        {
            return password != null && Encoding.UTF8.GetByteCount(password) <= RoomConstants.RoomPasswordLength;
        }

        private const string ipv4Regex =
            @"^(([1-9]?[0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])[.]){3}([1-9]?[0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])$";

        // https://community.helpsystems.com/forums/intermapper/miscellaneous-topics/5acc4fcf-fa83-e511-80cf-0050568460e4?_ga=2.113564423.1432958022.1523882681-2146416484.1523557976
        private const string ipv6Regex =
            @"^\s*((([0-9A-Fa-f]{1,4}:){7}([0-9A-Fa-f]{1,4}|:))|(([0-9A-Fa-f]{1,4}:){6}(:[0-9A-Fa-f]{1,4}|((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)(\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)){3})|:))|(([0-9A-Fa-f]{1,4}:){5}(((:[0-9A-Fa-f]{1,4}){1,2})|:((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)(\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)){3})|:))|(([0-9A-Fa-f]{1,4}:){4}(((:[0-9A-Fa-f]{1,4}){1,3})|((:[0-9A-Fa-f]{1,4})?:((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)(\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)){3}))|:))|(([0-9A-Fa-f]{1,4}:){3}(((:[0-9A-Fa-f]{1,4}){1,4})|((:[0-9A-Fa-f]{1,4}){0,2}:((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)(\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)){3}))|:))|(([0-9A-Fa-f]{1,4}:){2}(((:[0-9A-Fa-f]{1,4}){1,5})|((:[0-9A-Fa-f]{1,4}){0,3}:((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)(\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)){3}))|:))|(([0-9A-Fa-f]{1,4}:){1}(((:[0-9A-Fa-f]{1,4}){1,6})|((:[0-9A-Fa-f]{1,4}){0,4}:((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)(\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)){3}))|:))|(:(((:[0-9A-Fa-f]{1,4}){1,7})|((:[0-9A-Fa-f]{1,4}){0,5}:((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)(\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)){3}))|:)))(%.+)?\s*$";

        private const string urlRegex = @"^[A-Z,a-z,0-9,-,_]+(\.[A-Z,a-z,0-9,-,_]+)*$";
    }
}