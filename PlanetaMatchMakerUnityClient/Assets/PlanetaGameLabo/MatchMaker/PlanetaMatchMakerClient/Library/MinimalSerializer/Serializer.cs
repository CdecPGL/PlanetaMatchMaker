/*
The MIT License (MIT)

Copyright (c) 2019 Cdec

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Runtime.Serialization;
using System.Text;

namespace CdecPGL.MinimalSerializer
{
    public static class Serializer
    {
        public static int GetSerializedSize<T>()
        {
            return GetSerializedSizeImpl(typeof(T));
        }

        public static int GetSerializedSize(Type type)
        {
            return GetSerializedSizeImpl(type);
        }

        public static byte[] Serialize(object obj)
        {
            var size = GetSerializedSize(obj.GetType());
            var data = new byte[size];
            var pos = 0;
            SerializeImpl(obj, data, ref pos);
            return data;
        }

        public static T Deserialize<T>(byte[] source)
        {
            var size = GetSerializedSize(typeof(T));
            if (size != source.Length)
            {
                throw new InvalidSerializationException(
                    $"The size of source ({source.Length}) does not match the size of serialize target type ({typeof(T)}: {size})");
            }

            var pos = 0;
            DeserializeImpl(typeof(T), source, ref pos, out var obj);
            return (T)obj;
        }

        /// <summary>
        /// A set of directory serializable types.
        /// We don't use thread safe dictionary because this field will be only read.
        /// </summary>
        private static readonly HashSet<Type> directSerializableTypeSet = new HashSet<Type>()
        {
            typeof(bool),
            typeof(byte),
            typeof(sbyte),
            typeof(double),
            typeof(short),
            typeof(int),
            typeof(long),
            typeof(float),
            typeof(ushort),
            typeof(uint),
            typeof(ulong)
        };

        /// <summary>
        /// A directory serializable type to converter map.
        /// We don't use thread safe dictionary because this field will be only read.
        /// </summary>
        private static readonly Dictionary<Type, Func<byte[], int, object>> bytesToDirectSerializableTypeConverterDict
            = new Dictionary<Type, Func<byte[], int, object>>()
            {
                {typeof(bool), (bytes, startIdx) => BitConverter.ToBoolean(bytes, startIdx)},
                {typeof(byte), (bytes, startIdx) => bytes[startIdx]},
                {typeof(sbyte), (bytes, startIdx) => (sbyte)bytes[startIdx]},
                {typeof(double), (bytes, startIdx) => BitConverter.ToDouble(bytes, startIdx)},
                {typeof(short), (bytes, startIdx) => BitConverter.ToInt16(bytes, startIdx)},
                {typeof(int), (bytes, startIdx) => BitConverter.ToInt32(bytes, startIdx)},
                {typeof(long), (bytes, startIdx) => BitConverter.ToInt64(bytes, startIdx)},
                {typeof(float), (bytes, startIdx) => BitConverter.ToSingle(bytes, startIdx)},
                {typeof(ushort), (bytes, startIdx) => BitConverter.ToUInt16(bytes, startIdx)},
                {typeof(uint), (bytes, startIdx) => BitConverter.ToUInt32(bytes, startIdx)},
                {typeof(ulong), (bytes, startIdx) => BitConverter.ToUInt64(bytes, startIdx)},
            };

        /// <summary>
        /// Cache of serialized size for each type.
        /// We use thread safe dictionary because this field will be written from multi threads.
        /// </summary>
        private static readonly ConcurrentDictionary<Type, int> serializedSizeCache = new ConcurrentDictionary<Type, int>();

        private static int GetSerializedSizeImpl(Type type)
        {
            if (serializedSizeCache.ContainsKey(type))
            {
                return serializedSizeCache[type];
            }

            int size;
            if (IsDirectSerializableType(type))
            {
                size = GetSerializedSizeOfDirectSerializableType(type);
            }
            else if (IsFieldSerializableType(type))
            {
                throw new InvalidSerializationException(
                    $"The type ({type}) is serializable only when it is declared as a field");
            }
            else if (IsComplexSerializableType(type))
            {
                size = GetSerializedSizeOfComplexSerializableType(type);
            }
            else
            {
                ThrowNotSerializableException(type);
                throw new InvalidSerializationException("Logical error of serializable check.");
            }

            serializedSizeCache.TryAdd(type, size);
            return size;
        }

        private static int GetSerializedSizeOfDirectSerializableType(Type type)
        {
            // Return 1 if type is bool because Marshal.SizeOf returns 4 for bool due to C compability.
            if (type == typeof(bool))
            {
                return 1;
            }

            if (type.IsEnum)
            {
                type = type.GetEnumUnderlyingType();
            }

            return type == typeof(bool) ? 1 : Marshal.SizeOf(type);
        }

        private static int GetSerializedSizeOfFieldSerializableType(FieldInfo field, Type type)
        {
            // check cache here because this method doesn't called by GetSerializableSizeImpl, which checks cache
            if (serializedSizeCache.ContainsKey(type))
            {
                return serializedSizeCache[type];
            }

            int size;
            if (type == typeof(string))
            {
                size = GetLengthOfFixedLengthAttribute(field);
                // Not add to cache because the size is not fixed for same string type.
            }
            else if (field.FieldType.IsArray)
            {
                var length = GetLengthOfFixedLengthAttribute(field);
                var elementType = field.FieldType.GetElementType();
                size = GetSerializedSizeImpl(elementType) * length;
                // Not add to cache because the size is not fixed for same array type.
            }
            else
            {
                throw new InvalidSerializationException("Invalid type.");
            }

            return size;
        }

        private static int GetSerializedSizeOfComplexSerializableType(Type type)
        {
            var sum = 0;
            foreach (var field in GetFieldsOfComplexSerializableType(type))
            {
                if (IsFieldSerializableType(field.FieldType))
                {
                    sum += GetSerializedSizeOfFieldSerializableType(field, field.FieldType);
                }
                else
                {
                    sum += GetSerializedSizeImpl(field.FieldType);
                }
            }

            return sum;
        }

        private static void SerializeImpl(object obj, byte[] destination, ref int pos)
        {
            var type = obj.GetType();
            if (IsDirectSerializableType(type))
            {
                SerializeDirectSerializableType(obj, destination, ref pos);
            }
            else if (IsFieldSerializableType(type))
            {
                throw new InvalidSerializationException(
                    $"The type ({type}) is serializable only when it is declared as a field");
            }
            else if (IsComplexSerializableType(type))
            {
                SerializeComplexSerializableType(obj, destination, ref pos);
            }
            else
            {
                ThrowNotSerializableException(type);
                throw new InvalidSerializationException("Logical error of serializable check.");
            }
        }

        private static void SerializeDirectSerializableType(object obj, byte[] destination, ref int pos)
        {
            if (obj == null)
            {
                throw new InvalidSerializationException("Null reference is not serializable.");
            }

            var type = obj.GetType();
            if (type.IsEnum)
            {
                type = type.GetEnumUnderlyingType();
                obj = Convert.ChangeType(obj, type);
            }

            byte[] data;
            switch (obj)
            {
                case bool value:
                    data = BitConverter.GetBytes(value);
                    break;
                case byte value:
                    data = new[] {value};
                    break;
                case sbyte value:
                    data = new[] {(byte)value};
                    break;
                case double value:
                    data = BitConverter.GetBytes(value);
                    break;
                case short value:
                    data = BitConverter.GetBytes(value);
                    break;
                case int value:
                    data = BitConverter.GetBytes(value);
                    break;
                case long value:
                    data = BitConverter.GetBytes(value);
                    break;
                case float value:
                    data = BitConverter.GetBytes(value);
                    break;
                case ushort value:
                    data = BitConverter.GetBytes(value);
                    break;
                case uint value:
                    data = BitConverter.GetBytes(value);
                    break;
                case ulong value:
                    data = BitConverter.GetBytes(value);
                    break;
                default:
                    throw new InvalidSerializationException("Invalid type for serialization.");
            }

            if (BitConverter.IsLittleEndian)
            {
                Array.Reverse(data);
            }

            data.CopyTo(destination, pos);
            pos += GetSerializedSize(type);
        }

        private static void SerializeFieldSerializableType(object ownerObj, FieldInfo field, byte[] destination,
            ref int pos)
        {
            var obj = field.GetValue(ownerObj);
            if (obj == null)
            {
                throw new InvalidSerializationException(
                    $"Null reference is not serializable.({field.Name} in {ownerObj})");
            }

            if (field.FieldType == typeof(string))
            {
                var maxLength = GetLengthOfFixedLengthAttribute(field);
                // Encoding.UTF8.GetBytes doesn't include '\0' of end
                var data = Encoding.UTF8.GetBytes((string)obj);
                if (data.Length > maxLength)
                {
                    throw new InvalidSerializationException(
                        $"The length of string ({data.Length}) exceeds max length indicated by attribute ({maxLength}).");
                }

                for (var i = 0; i < maxLength; ++i)
                {
                    destination[pos + i] = i < data.Length ? data[i] : (byte)'\0';
                }

                pos += maxLength;
            }
            else if (field.FieldType.IsArray)
            {
                var length = GetLengthOfFixedLengthAttribute(field);
                var array = (Array)obj;
                if (array.Length != length)
                {
                    throw new InvalidSerializationException(
                        $"The size of array ({array.Length}) does not match the size indicated by attribute ({length}).");
                }

                for (var i = 0; i < length; ++i)
                {
                    SerializeImpl(array.GetValue(i), destination, ref pos);
                }
            }
            else
            {
                throw new InvalidSerializationException("Invalid type.");
            }
        }

        private static void SerializeComplexSerializableType(object obj, byte[] destination, ref int pos)
        {
            if (obj == null)
            {
                throw new InvalidSerializationException("Null reference is not serializable.");
            }

            var type = obj.GetType();
            foreach (var field in GetFieldsOfComplexSerializableType(type))
            {
                if (IsFieldSerializableType(field.FieldType))
                {
                    SerializeFieldSerializableType(obj, field, destination, ref pos);
                }
                else
                {
                    SerializeImpl(field.GetValue(obj), destination, ref pos);
                }
            }
        }

        private static void DeserializeImpl(Type type, byte[] source, ref int pos, out object obj)
        {
            if (IsDirectSerializableType(type))
            {
                DeserializeDirectSerializableType(type, source, ref pos, out obj);
            }
            else if (IsFieldSerializableType(type))
            {
                throw new InvalidSerializationException(
                    $"The type ({type}) is serializable only when it is declared as a field");
            }
            else if (IsComplexSerializableType(type))
            {
                DeserializeComplexSerializableType(type, source, ref pos, out obj);
            }
            else
            {
                ThrowNotSerializableException(type);
                throw new InvalidSerializationException("Logical error of serializable check.");
            }
        }

        private static void DeserializeDirectSerializableType(Type type, byte[] source, ref int pos,
            out object obj)
        {
            var size = GetSerializedSize(type);
            if (BitConverter.IsLittleEndian)
            {
                Array.Reverse(source, pos, size);
            }

            if (type.IsEnum)
            {
                type = type.GetEnumUnderlyingType();
            }

            try
            {
                if (bytesToDirectSerializableTypeConverterDict.ContainsKey(type))
                {
                    obj = bytesToDirectSerializableTypeConverterDict[type](source, pos);
                }
                else
                {
                    throw new InvalidSerializationException("Invalid type for serialization.");
                }
            }
            finally
            {
                // Make source to first status
                if (BitConverter.IsLittleEndian)
                {
                    Array.Reverse(source, pos, size);
                }
            }

            pos += size;
        }

        private static void DeserializeFieldSerializableType(object ownerObj, FieldInfo field, byte[] source,
            ref int pos)
        {
            object obj;
            if (field.FieldType == typeof(string))
            {
                var maxLength = GetLengthOfFixedLengthAttribute(field);
                var realLength = Array.IndexOf(source, (byte)'\0', pos, maxLength) - pos;
                if (realLength < 0)
                {
                    realLength = maxLength;
                }

                obj = Encoding.UTF8.GetString(source, pos, realLength);
                pos += maxLength;
            }
            else if (field.FieldType.IsArray)
            {
                var length = GetLengthOfFixedLengthAttribute(field);
                obj = Activator.CreateInstance(field.FieldType, length);
                var array = (Array)obj;
                var elementType = field.FieldType.GetElementType();
                for (var i = 0; i < length; ++i)
                {
                    DeserializeImpl(elementType, source, ref pos, out var elementObj);
                    array.SetValue(elementObj, i);
                }
            }
            else
            {
                throw new InvalidSerializationException("Invalid type.");
            }

            field.SetValue(ownerObj, obj);
        }

        private static void DeserializeComplexSerializableType(Type type, byte[] source, ref int pos, out object obj)
        {
            obj = FormatterServices.GetUninitializedObject(type);

            foreach (var field in GetFieldsOfComplexSerializableType(type))
            {
                if (IsFieldSerializableType(field.FieldType))
                {
                    DeserializeFieldSerializableType(obj, field, source, ref pos);
                }
                else
                {
                    DeserializeImpl(field.FieldType, source, ref pos, out var fieldObj);
                    field.SetValue(obj, fieldObj);
                }
            }
        }

        private static int GetLengthOfFixedLengthAttribute(FieldInfo field)
        {
            var fixedLengthAttribute = field.GetCustomAttribute<FixedLengthAttribute>();
            if (fixedLengthAttribute == null)
            {
                throw new InvalidSerializationException("There is no FixedLengthAttribute set to the field.");
            }

            return fixedLengthAttribute.Length;
        }

        private static bool IsDirectSerializableType(Type type)
        {
            return directSerializableTypeSet.Contains(type) || type.IsEnum;
        }

        private static bool IsFieldSerializableType(Type type)
        {
            return type == typeof(string) || type.IsArray;
        }

        private static bool IsComplexSerializableType(Type type)
        {
            return type.IsLayoutSequential && type.IsSerializable && GetFieldsOfComplexSerializableType(type).Length > 0;
        }

        private static void ThrowNotSerializableException(Type type)
        {
            throw new InvalidSerializationException(
                $"The type ({type}) is not serializable. Primitive types, fixed string, fixed array and class (struct) which is sequential and serializable are available.");
        }

        private static FieldInfo[] GetFieldsOfComplexSerializableType(Type type)
        {
            // If not sequential, Marshal is not available and throws an exception
            Debug.Assert(type.IsLayoutSequential);
            // GetFields() returns inconsistent order even if LayoutKind.Sequential is true in some case (I cannot find detailed condition...).
            // To avoid this, order by MetadataToken but Marshal.OffsetOf(Type, string) is deduplicate according to the document...
            // https://stackoverflow.com/questions/8067493/if-getfields-doesnt-guarantee-order-how-does-layoutkind-sequential-work
            return type.GetFields(BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.Instance)
                .OrderBy(f => Marshal.OffsetOf(type, f.Name).ToInt64()).ToArray();
            // This is also available solution but this behavior is not documented.
            //return type.GetFields(BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.Instance)
            //    .OrderBy(field => field.MetadataToken).ToArray();
        }
    }
}