using System;
using System.Collections.Generic;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Text;

namespace PlanetaGameLabo.Serializer {
    public static class Serializer {
        public static int GetSerializedSize<T>() {
            return GetSerializedSizeImpl(typeof(T));
        }

        public static int GetSerializedSize(Type type) {
            return GetSerializedSizeImpl(type);
        }

        public static byte[] Serialize(object obj) {
            var size = GetSerializedSize(obj.GetType());
            var data = new byte[size];
            var pos = 0;
            SerializeImpl(obj, data, ref pos);
            return data;
        }

        public static T Deserialize<T>(byte[] source) {
            var size = GetSerializedSize(typeof(T));
            if (size != source.Length) {
                throw new InvalidSerializationException(
                    $"The size of source ({source.Length}) does not match the size of serialize target type ({typeof(T)}: {size})");
            }

            var pos = 0;
            DeserializeImpl(typeof(T), source, ref pos, out var obj);
            return (T) obj;
        }

        private static readonly HashSet<Type> _directSerializableTypeSet = new HashSet<Type>() {
            typeof(bool), typeof(byte), typeof(sbyte),
            typeof(double), typeof(short), typeof(int), typeof(long),
            typeof(float), typeof(ushort), typeof(uint), typeof(ulong)
        };

        private static readonly Dictionary<Type, Func<byte[], int, object>> _bytesToDirectSerializableTypeConverterDict
            = new Dictionary<Type, Func<byte[], int, object>>() {
                {typeof(bool), (bytes, start_idx) => BitConverter.ToBoolean(bytes, start_idx)},
                {typeof(byte), (bytes, start_idx) => bytes[0]},
                {typeof(sbyte), (bytes, start_idx) => (sbyte) bytes[0]},
                {typeof(double), (bytes, start_idx) => BitConverter.ToDouble(bytes, start_idx)},
                {typeof(short), (bytes, start_idx) => BitConverter.ToInt16(bytes, start_idx)},
                {typeof(int), (bytes, start_idx) => BitConverter.ToInt32(bytes, start_idx)},
                {typeof(long), (bytes, start_idx) => BitConverter.ToInt64(bytes, start_idx)},
                {typeof(float), (bytes, start_idx) => BitConverter.ToSingle(bytes, start_idx)},
                {typeof(ushort), (bytes, start_idx) => BitConverter.ToUInt16(bytes, start_idx)},
                {typeof(uint), (bytes, start_idx) => BitConverter.ToUInt32(bytes, start_idx)},
                {typeof(ulong), (bytes, start_idx) => BitConverter.ToUInt64(bytes, start_idx)},
            };

        private static readonly Dictionary<Type, int> _serializedSizeCache = new Dictionary<Type, int>();

        private static int GetSerializedSizeImpl(Type type) {
            if (_serializedSizeCache.ContainsKey(type)) {
                return _serializedSizeCache[type];
            }

            int size;
            if (IsDirectSerializableType(type)) {
                size = GetSerializedSizeOfDirectSerializableType(type);
            }
            else if (IsFieldSerializableType(type)) {
                throw new InvalidSerializationException(
                    $"The type ({type}) is serializable only when it is declared as a field");
            }
            else if (IsComplexSerializableType(type)) {
                size = GetSerializedSizeOfComplexSerializableType(type);
            }
            else {
                throw new InvalidSerializationException(
                    $"The type ({type}) is not serializable. Primitive types, fixed string and class (struct) which is sequential and serializable are available.");
            }

            _serializedSizeCache.Add(type, size);
            return size;
        }

        private static int GetSerializedSizeOfDirectSerializableType(Type type) {
            // Return 1 if type is bool because Marshal.SizeOf returns 4 for bool due to C compability.
            return type == typeof(bool) ? 1 : Marshal.SizeOf(type);
        }

        private static int GetSerializedSizeOfFieldSerializableType(FieldInfo field, Type type) {
            // check cache here because this method doesn't called by GetSerializableSizeImpl, which checks cache
            if (_serializedSizeCache.ContainsKey(type)) {
                return _serializedSizeCache[type];
            }

            int size;
            if (type == typeof(string)) {
                size = GetLengthOfFixedLengthAttribute(field);
                // Not add to cache because the size is not fixed for same string type.
            }
            else if (field.FieldType.IsArray) {
                var length = GetLengthOfFixedLengthAttribute(field);
                var element_type = field.FieldType.GetElementType();
                size = GetSerializedSizeImpl(element_type) * length;
                // Not add to cache because the size is not fixed for same array tipe.
            }
            else {
                throw new InvalidSerializationException("Invalid type.");
            }

            return size;
        }

        private static int GetSerializedSizeOfComplexSerializableType(Type type) {
            var sum = 0;
            foreach (var field in type.GetFields(BindingFlags.Public | BindingFlags.NonPublic |
                                                 BindingFlags.Instance)) {
                if (IsFieldSerializableType(field.FieldType)) {
                    sum += GetSerializedSizeOfFieldSerializableType(field, field.FieldType);
                }
                else {
                    sum += GetSerializedSizeImpl(field.FieldType);
                }
            }

            return sum;
        }

        private static void SerializeImpl(object obj, byte[] destination, ref int pos) {
            var type = obj.GetType();
            if (IsDirectSerializableType(type)) {
                SerializeDirectSerializableType(obj, destination, ref pos);
            }
            else if (IsFieldSerializableType(type)) {
                throw new InvalidSerializationException(
                    $"The type ({type}) is serializable only when it is declared as a field");
            }
            else if (IsComplexSerializableType(type)) {
                SerializeComplexSerializableType(obj, destination, ref pos);
            }
            else {
                throw new InvalidSerializationException(
                    $"The type ({type}) is not serializable. Primitive types, fixed string and class (struct) which is sequential and serializable are available.");
            }
        }

        private static void SerializeDirectSerializableType(object obj, byte[] destination, ref int pos) {
            var type = obj.GetType();
            byte[] data;
            switch (obj) {
                case bool value:
                    data = BitConverter.GetBytes(value);
                    break;
                case byte value:
                    data = new[] {value};
                    break;
                case sbyte value:
                    data = new[] {(byte) value};
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

            if (BitConverter.IsLittleEndian) {
                Array.Reverse(data);
            }

            data.CopyTo(destination, pos);
            pos += GetSerializedSize(type);
        }

        private static void
            SerializeFieldSerializableType(object owner_obj, FieldInfo field, byte[] destination, ref int pos) {
            var obj = field.GetValue(owner_obj);
            if (field.FieldType == typeof(string)) {
                var max_length = GetLengthOfFixedLengthAttribute(field);
                // Encoding.UTF8.GetBytes doesn't include '\0' of end
                var data = Encoding.UTF8.GetBytes((string) obj);
                if (data.Length > max_length) {
                    throw new InvalidSerializationException(
                        $"The length of string ({data.Length}) exceeds max length indicated by attribute ({max_length}).");
                }

                for (var i = 0; i < max_length; ++i) {
                    destination[pos + i] = i < data.Length ? data[i] : (byte) '\0';
                }

                pos += max_length;
            }
            else if (field.FieldType.IsArray) {
                var length = GetLengthOfFixedLengthAttribute(field);
                var array = (Array) obj;
                if (array.Length != length) {
                    throw new InvalidSerializationException(
                        $"The size of array ({array.Length}) does not match the size indicated by attribute ({length}).");
                }

                for (var i = 0; i < length; ++i) {
                    SerializeImpl(array.GetValue(i), destination, ref pos);
                }
            }
            else {
                throw new InvalidSerializationException("Invalid type.");
            }
        }

        private static void SerializeComplexSerializableType(object obj, byte[] destination, ref int pos) {
            var type = obj.GetType();
            foreach (var field in type.GetFields(BindingFlags.Public | BindingFlags.NonPublic |
                                                 BindingFlags.Instance)
            ) {
                if (IsFieldSerializableType(field.FieldType)) {
                    SerializeFieldSerializableType(obj, field, destination, ref pos);
                }
                else {
                    SerializeImpl(field.GetValue(obj), destination, ref pos);
                }
            }
        }

        private static void DeserializeImpl(Type type, byte[] source, ref int pos, out object obj) {
            if (IsDirectSerializableType(type)) {
                DeserializeDirectSerializableType(type, source, ref pos, out obj);
            }
            else if (IsFieldSerializableType(type)) {
                throw new InvalidSerializationException(
                    $"The type ({type}) is serializable only when it is declared as a field");
            }
            else if (IsComplexSerializableType(type)) {
                DeserializeComplexSerializableType(type, source, ref pos, out obj);
            }
            else {
                throw new InvalidSerializationException(
                    $"The type ({type}) is not serializable. Primitive types, fixed string and class (struct) which is sequential and serializable are available.");
            }
        }

        private static void DeserializeDirectSerializableType(Type type, byte[] source, ref int pos,
            out object obj) {
            var size = GetSerializedSize(type);
            if (BitConverter.IsLittleEndian) {
                Array.Reverse(source, pos, size);
            }

            try {
                if (_bytesToDirectSerializableTypeConverterDict.ContainsKey(type)) {
                    obj = _bytesToDirectSerializableTypeConverterDict[type](source, pos);
                }
                else {
                    throw new InvalidSerializationException("Invalid type for serialization.");
                }
            }
            finally {
                // Make source to first status
                if (BitConverter.IsLittleEndian) {
                    Array.Reverse(source, pos, size);
                }
            }

            pos += size;
        }

        private static void DeserializeFieldSerializableType(object owner_obj, FieldInfo field, byte[] source,
            ref int pos) {
            object obj;
            if (field.FieldType == typeof(string)) {
                var max_length = GetLengthOfFixedLengthAttribute(field);
                var real_length = Array.IndexOf(source, (byte) '\0', pos, max_length) - pos;
                if (real_length < 0) {
                    real_length = max_length;
                }

                obj = Encoding.UTF8.GetString(source, pos, real_length);
                pos += max_length;
            }
            else if (field.FieldType.IsArray) {
                var length = GetLengthOfFixedLengthAttribute(field);
                obj = Activator.CreateInstance(field.FieldType, length);
                var array = (Array) obj;
                var element_type = field.FieldType.GetElementType();
                for (var i = 0; i < length; ++i) {
                    DeserializeImpl(element_type, source, ref pos, out var element_obj);
                    array.SetValue(element_obj, i);
                }
            }
            else {
                throw new InvalidSerializationException("Invalid type.");
            }

            field.SetValue(owner_obj, obj);
        }

        private static void
            DeserializeComplexSerializableType(Type type, byte[] source, ref int pos, out object obj) {
            obj = Activator.CreateInstance(type);

            foreach (var field in type.GetFields(BindingFlags.Public | BindingFlags.NonPublic |
                                                 BindingFlags.Instance)
            ) {
                if (IsFieldSerializableType(field.FieldType)) {
                    DeserializeFieldSerializableType(obj, field, source, ref pos);
                }
                else {
                    DeserializeImpl(field.FieldType, source, ref pos, out var field_obj);
                    field.SetValue(obj, field_obj);
                }
            }
        }

        private static int GetLengthOfFixedLengthAttribute(FieldInfo field) {
            var fixed_length_attribute = field.GetCustomAttribute<FixedLengthAttribute>();
            if (fixed_length_attribute == null) {
                throw new InvalidSerializationException("There is no FixedLengthAttribute set to the field.");
            }

            return fixed_length_attribute.length;
        }

        private static bool IsDirectSerializableType(Type type) {
            return _directSerializableTypeSet.Contains(type);
        }

        private static bool IsFieldSerializableType(Type type) {
            return type == typeof(string) || type.IsArray;
        }

        private static bool IsComplexSerializableType(Type type) {
            return type.GetFields().Length > 0 && type.IsLayoutSequential && type.IsSerializable;
        }
    }
}