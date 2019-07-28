using System;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using PlanetaGameLabo;
using PlanetaMatchMakerClient;

namespace PlanetaGameLabo.Test {
    [Serializable]
    struct FixedStringStruct {
        [FixedLength(16)] public string str;
    }

    [Serializable]
    struct FixedArrayStruct {
        [FixedLength(32)] public int[] a;
    }

    [Serializable]
    struct TestStruct {
        public short a;
        public uint b;
        public double c;
        [FixedLength(16)] public string str;
        [FixedLength(32)] public int[] array;
    }

    [Serializable]
    struct NestedTestStruct {
        public uint a;
        public double b;
        public TestStruct c;
    }

    struct NotSerializableStruct {
        public int a;
    }

    [Serializable]
    struct NotFixedStringStruct {
        public string str;
    }

    [Serializable]
    struct NotFixedArrayStruct {
        public int[] a;
    }


    [TestClass]
    public class SerializerTest {
        [TestCategory("ClientSerializer.GetSerializedSize")]
        [TestMethod]
        public void GetSerializedSizeByteTest() {
            Assert.AreEqual(1, Serializer.GetSerializedSize<byte>());
        }

        [TestCategory("ClientSerializer.GetSerializedSize")]
        [TestMethod]
        public void GetSerializedSizeShortTest() {
            Assert.AreEqual(2, Serializer.GetSerializedSize<short>());
        }

        [TestCategory("ClientSerializer.GetSerializedSize")]
        [TestMethod]
        public void GetSerializedSizeLongTest() {
            Assert.AreEqual(8, Serializer.GetSerializedSize<long>());
        }

        [TestCategory("ClientSerializer.GetSerializedSize")]
        [TestMethod]
        public void GetSerializedSizeFixedStringTest() {
            Assert.AreEqual(16, Serializer.GetSerializedSize<FixedStringStruct>());
        }

        [TestCategory("ClientSerializer.GetSerializedSize")]
        [TestMethod]
        public void GetSerializedSizeFixedArrayTest() {
            Assert.AreEqual(128, Serializer.GetSerializedSize<FixedArrayStruct>());
        }

        [TestCategory("ClientSerializer.GetSerializedSize")]
        [TestMethod]
        public void GetSerializedSizeStructTest() {
            Assert.AreEqual(158, Serializer.GetSerializedSize<TestStruct>());
        }

        [TestCategory("ClientSerializer.GetSerializedSize")]
        [TestMethod]
        public void GetSerializedSizeNestedStructTest() {
            Assert.AreEqual(170, Serializer.GetSerializedSize<NestedTestStruct>());
        }

        [TestCategory("ClientSerializer.GetSerializedSize")]
        [ExpectedException(typeof(InvalidSerializationException))]
        [TestMethod]
        public void GetSerializedSizeNotFixedStringTest() {
            Serializer.GetSerializedSize<NotFixedStringStruct>();
        }

        [TestCategory("ClientSerializer.GetSerializedSize")]
        [ExpectedException(typeof(InvalidSerializationException))]
        [TestMethod]
        public void GetSerializedSizeNotFixedArrayTest() {
            Serializer.GetSerializedSize<NotFixedArrayStruct>();
        }

        [TestCategory("ClientSerializer.GetSerializedSize")]
        [ExpectedException(typeof(InvalidSerializationException))]
        [TestMethod]
        public void GetSerializedSizeNotSerializableStruct() {
            Serializer.GetSerializedSize<NotSerializableStruct>();
        }
    }
}