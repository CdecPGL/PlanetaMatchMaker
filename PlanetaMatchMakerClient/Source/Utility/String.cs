using System;
using System.Text;

namespace PlanetaGameLabo.MatchMaker
{
    public static class Utility
    {
        /// <summary>
        /// Convert a string to indicated size array.
        /// </summary>
        /// <param name="source">A string to convert.</param>
        /// <param name="arrayLength">A length of result array.</param>
        /// <remarks>TODO: Move to minimal serializer</remarks>
        /// <returns></returns>
        /// <exception cref="ArgumentException">The byte length of the string exceeds the length of array.</exception>
        public static byte[] ConvertStringToFixedLengthArray(string source, int arrayLength)
        {
            var rawArray = Encoding.UTF8.GetBytes(source);
            if (rawArray.Length > arrayLength)
            {
                throw new ArgumentException(
                    $"The byte length of string ({rawArray.Length}) exceeds max length ({arrayLength}).");
            }

            // Add '\0' if need
            var destination = new byte[Math.Min(arrayLength, rawArray.Length + 1)];
            for (var i = 0; i < arrayLength && i < rawArray.Length + 1; ++i)
            {
                destination[i] = i < rawArray.Length ? rawArray[i] : (byte)'\0';
            }

            return destination;
        }

        /// <summary>
        /// Convert a fixed length byte array to string.
        /// </summary>
        /// <param name="source">A byte array to convert.</param>
        /// <remarks>TODO: Move to minimal serializer</remarks>
        /// <returns></returns>
        /// <exception cref="ArgumentException">Failed to decode as UTF-8.</exception>
        /// <exception cref="ArgumentNullException">Any of arguments are null.</exception>
        public static string ConvertFixedLengthArrayToString(byte[] source)
        {
            if (source == null)
            {
                throw new ArgumentNullException(nameof(source));
            }

            var realLength = Array.IndexOf(source, (byte)'\0', 0);
            if (realLength < 0)
            {
                realLength = source.Length;
            }

            try
            {
                return Encoding.UTF8.GetString(source, 0, realLength);
            }
            catch (DecoderFallbackException e)
            {
                throw new AggregateException(e.Message);
            }
            catch (ArgumentOutOfRangeException e)
            {
                throw new AggregateException(e.Message);
            }
        }
    }
}