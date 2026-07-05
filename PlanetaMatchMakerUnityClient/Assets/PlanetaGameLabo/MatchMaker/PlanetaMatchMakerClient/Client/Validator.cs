namespace PlanetaGameLabo.MatchMaker
{
    public static class Validator
    {
        /// <summary>
        /// Validate server address.
        /// IPv4, IPv6 or host name is valid.
        /// </summary>
        /// <param name="address"></param>
        /// <returns></returns>
        public static bool ValidateServerAddress(string address)
        {
            return MatchMakerServerAddress.IsValid(address);
        }

        /// <summary>
        /// Validate server port.
        /// Port which is not 0 is valid.
        /// </summary>
        /// <param name="port"></param>
        /// <returns></returns>
        public static bool ValidateServerPort(ushort port)
        {
            return MatchMakerServerPort.IsValid(port);
        }

        /// <summary>
        /// Validate player name.
        /// Not null or space string whose length is not more than limit is valid.
        /// </summary>
        /// <param name="name"></param>
        /// <returns></returns>
        public static bool ValidatePlayerName(string name)
        {
            return PlayerName.IsValid(name);
        }

        /// <summary>
        /// Validate a game hot port.
        /// dynamic/private port is valid.
        /// </summary>
        /// <param name="port"></param>
        /// <returns></returns>
        public static bool ValidateGameHostPort(ushort port)
        {
            return GameHostPort.IsValid(port);
        }

        /// <summary>
        /// Validate a room game host external id.
        /// Not null string whose length is not more than limit is valid.
        /// </summary>
        /// <param name="externalId"></param>
        /// <returns></returns>
        public static bool ValidateGameHostExternalId(byte[] externalId)
        {
            return GameHostExternalId.IsValid(externalId);
        }

        /// <summary>
        /// Validate a room password.
        /// Not null string whose length is not more than limit is valid.
        /// </summary>
        /// <param name="password"></param>
        /// <returns></returns>
        public static bool ValidateRoomPassword(string password)
        {
            return RoomPassword.IsValid(password);
        }

        /// <summary>
        /// Validate a search name.
        /// Not null string whose length is not more than limit is valid.
        /// </summary>
        /// <param name="name"></param>
        /// <returns></returns>
        public static bool ValidateSearchName(string name)
        {
            return SearchName.IsValid(name);
        }

    }
}
