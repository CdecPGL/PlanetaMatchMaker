using System;
using System.Net;

namespace PlanetaGameLabo.MatchMaker
{
    public readonly struct RoomHostEndpoint
    {
        public RoomHostEndpoint(GameHostSignalingMethod signalingMethod, IPEndPoint ipEndpoint, byte[] externalId)
        {
            SignalingMethod = signalingMethod;
            IPEndpoint = ipEndpoint;
            ExternalId = externalId;

            try
            {
                ExternalIdString = Utility.ConvertFixedLengthArrayToString(ExternalId);
            }
            catch (ArgumentException)
            {
                ExternalIdString = null;
            }
        }

        /// <summary>
        /// A signaling method to connect to the host.
        /// </summary>
        public readonly GameHostSignalingMethod SignalingMethod;

        /// <summary>
        /// An IP endpoint of the game host when signaling method is direct.
        /// </summary>
        public readonly IPEndPoint IPEndpoint;

        /// <summary>
        /// An external service ID of game host when signaling method is external service.
        /// </summary>
        public readonly byte[] ExternalId;

        /// <summary>
        /// An string which is generated from the external service ID as UTF-8 of game host when signaling method is external service.
        /// This property will be null if external service ID is not UTF-8 string.
        /// </summary>
        public readonly string ExternalIdString;
    }
}