/*
The MIT License (MIT)

Copyright (c) 2020 Cdec

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace PlanetaGameLabo.MatchMaker.Test
{
    [TestClass]
    [TestCategory("Validator")]
    public class ValidatorTest
    {
        [DataTestMethod]
        [DataRow("127.0.0.1")]
        [DataRow("0.0.0.0")]
        [DataRow("255.255.255.255")]
        [DataRow("2001:0db8:bd05:01d2:288a:1fc0:0001:10ee")]
        [DataRow("2001:db8::9abc")]
        [DataRow("a.b.c")]
        [DataRow("a")]
        public void ValidServerAddressValidationTest(string address)
        {
            Assert.IsTrue(Validator.ValidateServerAddress(address));
        }

        [DataTestMethod]
        [DataRow("2001:0db8:bd05:01d2:288a:1fc0:0001:10ee:ffff")]
        [DataRow("2001::db8::9abc")]
        [DataRow(".a.b.c")]
        [DataRow("a.b.c.")]
        [DataRow("a.b..c")]
        [DataRow(".")]
        [DataRow("")]
        [DataRow(null)]
        public void InvalidServerAddressValidationTest(string address)
        {
            Assert.IsFalse(Validator.ValidateServerAddress(address));
        }

        [DataTestMethod]
        [DataRow((ushort)1)]
        [DataRow((ushort)65535)]
        public void ValidServerPortValidationTest(ushort port)
        {
            Assert.IsTrue(Validator.ValidateServerPort(port));
        }

        [DataTestMethod]
        [DataRow((ushort)0)]
        public void InvalidServerPortValidationTest(ushort port)
        {
            Assert.IsFalse(Validator.ValidateServerPort(port));
        }

        [DataTestMethod]
        [DataRow("p")]
        [DataRow("123456789012345678901234")]
        [DataRow("ああああああああ")]
        [DataRow("あああああああaaa")]
        public void ValidPlayerNameValidationTest(string playerName)
        {
            Assert.IsTrue(Validator.ValidatePlayerName(playerName));
        }

        [DataTestMethod]
        [DataRow(null)]
        [DataRow("")]
        [DataRow("1234567890123456789012345")]
        [DataRow("あああああああああ")]
        [DataRow("ああああああああaaa")]
        [DataRow("あああああああaaaa")]
        public void InvalidPlayerNameValidationTest(string playerName)
        {
            Assert.IsFalse(Validator.ValidatePlayerName(playerName));
        }

        [DataTestMethod]
        [DataRow((ushort)49152)]
        [DataRow((ushort)65535)]
        public void ValidGameHostPortValidationTest(ushort port)
        {
            Assert.IsTrue(Validator.ValidateGameHostPort(port));
        }

        [DataTestMethod]
        [DataRow((ushort)0)]
        [DataRow((ushort)49151)]
        public void InvalidGameHostPortValidationTest(ushort port)
        {
            Assert.IsFalse(Validator.ValidateGameHostPort(port));
        }

        [DataTestMethod]
        [DataRow("")]
        [DataRow("p")]
        [DataRow("1234567890123456")]
        [DataRow("あああああ")]
        [DataRow("あああああa")]
        public void ValidRoomPasswordValidationTest(string password)
        {
            Assert.IsTrue(Validator.ValidateRoomPassword(password));
        }

        [DataTestMethod]
        [DataRow(null)]
        [DataRow("1234567890123456789012345")]
        [DataRow("12345678901234567")]
        [DataRow("ああああああ")]
        [DataRow("あああああaa")]
        public void InvalidRoomPasswordValidationTest(string password)
        {
            Assert.IsFalse(Validator.ValidateRoomPassword(password));
        }
    }
}