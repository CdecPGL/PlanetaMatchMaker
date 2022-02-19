using System;
using System.Globalization;
using System.Linq;
using UnityEngine;
#if UNITY_EDITOR
using CdecPGL.MinimalSerializer;
using UnityEditor;

#endif

namespace PlanetaGameLabo.MatchMaker
{
    /**
     * A type which represents fixed size byte array which is displayed in inspector.
     */
    [Serializable]
    public sealed class FixedSizeByteArray
    {
        /// <summary>
        /// The length of byte array.
        /// </summary>
        public int length { get; }

        /// <summary>
        /// A byte array.
        /// </summary>
        public byte[] value => _value;

        /// <summary>
        /// Constructor.
        /// </summary>
        /// <param name="length">The length of byte array.</param>
        public FixedSizeByteArray(int length)
        {
            this.length = length;
        }

        [SerializeField] private byte[] _value = Array.Empty<byte>();
    }

#if UNITY_EDITOR
    /// <summary>
    /// Drawer of FixedSizeByteArrayDrawer
    /// </summary>
    [CustomPropertyDrawer(typeof(FixedSizeByteArray))]
    public sealed class FixedSizeByteArrayDrawer : PropertyDrawer
    {
        private float _startKey;
        private float _startValue;
        private const float _lineHeight = 16;
        private const float _linePadding = 2;
        private bool _isFoldout;
        private bool _isKeyValueInitialized;

        public override void OnGUI(Rect position, SerializedProperty property, GUIContent label)
        {
            // Check property type
            if (fieldInfo.FieldType != typeof(FixedSizeByteArray))
            {
                Debug.LogError($"The target must be {nameof(FixedSizeByteArray)}.");
                return;
            }

            var parentComponent = property.serializedObject.targetObject;
            var fixedSizeByteArray = (FixedSizeByteArray)fieldInfo.GetValue(parentComponent);

            // Get TooltipAttribute content if exist
            string tooltipStr = "A byte array.";
            var tooltipAttribute = fieldInfo.GetCustomAttributes(typeof(TooltipAttribute), true);
            if (tooltipAttribute.Length > 0)
            {
                tooltipStr = ((TooltipAttribute)tooltipAttribute[0]).tooltip;
            }


            var byteArrayObject = property.FindPropertyRelative("_value");

            // Prepare byte array
            var arrayLength = fixedSizeByteArray.length;
            byte[] newByteArray = null;

            // Get current byte array
            var byteArray = new byte[arrayLength];
            var currentArrayLength = byteArrayObject.arraySize;
            for (var i = 0; i < arrayLength; ++i)
            {
                var idx = arrayLength - i - 1;
                var currentArrayIdx = currentArrayLength - i - 1;
                if (currentArrayIdx >= 0)
                {
                    var element = byteArrayObject.GetArrayElementAtIndex(currentArrayIdx);
                    byteArray[idx] = (byte)element.intValue;
                }
                else
                {
                    byteArray[idx] = 0;
                }
            }

            int line = 0;
            string labelText = label.text;

            EditorGUI.BeginProperty(position, label, property);

            EditorGUI.LabelField(GetSeparatedRect(position, line, 3, 0), new GUIContent(labelText, tooltipStr));

            ++EditorGUI.indentLevel;

            _isFoldout = EditorGUI.Foldout(GetSeparatedRect(position, line, 3, 1, 2), _isFoldout, "Detail", true);
            ++line;

            // Display if it is not folded
            if (_isFoldout)
            {
                // Array Size
                {
                    EditorGUI.LabelField(GetSeparatedRect(position, line, 3, 0), "Size");
                    EditorGUI.LabelField(GetSeparatedRect(position, line, 3, 1, 2), $"{arrayLength}");
                    ++line;
                }

                // Hex Binary
                {
                    EditorGUI.LabelField(GetSeparatedRect(position, line, 3, 0), "Hex Binary");
                    var str = string.Join("", byteArray.Select(b => b.ToString("X2")));
                    var newStr = EditorGUI.TextField(GetSeparatedRect(position, line, 3, 1, 2), str);
                    if (str != newStr)
                    {
                        if (newStr.Length > arrayLength * 2)
                        {
                            // Trim larger index elements
                            newStr = newStr.Substring(newStr.Length - arrayLength * 2, arrayLength * 2);
                        }
                        else if (newStr.Length < arrayLength * 2)
                        {
                            // Add larger index elements
                            newStr = newStr.PadRight(arrayLength * 2, '0');
                        }

                        newByteArray = new byte[arrayLength];
                        try
                        {
                            for (var i = 0; i < arrayLength; ++i)
                            {
                                var b = byte.Parse(newStr.Substring(i * 2, 2), NumberStyles.HexNumber);
                                newByteArray[i] = b;
                            }
                        }
                        catch (FormatException) { }
                    }

                    ++line;
                }

                // UTF-8 String
                {
                    EditorGUI.LabelField(GetSeparatedRect(position, line, 3, 0), "UTF-8 String");
                    var str = "";
                    try
                    {
                        str = Utility.ConvertFixedLengthArrayToString(byteArray);
                    }
                    catch (ArgumentException) { }

                    var newStr = EditorGUI.TextField(GetSeparatedRect(position, line, 3, 1, 2), str);
                    if (str != newStr)
                    {
                        try
                        {
                            var newRawByteArray = Utility.ConvertStringToFixedLengthArray(newStr, arrayLength);
                            newByteArray = new byte[arrayLength];
                            Array.Copy(newRawByteArray, newByteArray, newRawByteArray.Length);
                        }
                        catch (ArgumentException) { }
                    }

                    ++line;
                }

                // Unsigned Integer (Big Endien)
                {
                    void Integer<T>(string fieldLabel, int intSize, Func<string, T> parser)
                    {
                        EditorGUI.LabelField(GetSeparatedRect(position, line, 3, 0), fieldLabel);
                        var trimmedByteArray = new byte[intSize];
                        if (arrayLength >= intSize)
                        {
                            Array.Copy(byteArray, 0, trimmedByteArray, 0, intSize);
                        }
                        else
                        {
                            // Use smaller bits to be compatible with int32, int16, etc.
                            Array.Copy(byteArray, 0, trimmedByteArray, intSize - arrayLength, arrayLength);
                        }

                        var integer = Serializer.Deserialize<T>(trimmedByteArray);
                        // Use string to display in inspector because EditorGUI doesn't have field method for ulong. 
                        var integerString = integer.ToString();
                        var newIntegerString = EditorGUI.TextField(GetSeparatedRect(position, line, 3, 1, 2),
                            integerString,
                            EditorStyles.numberField);
                        if (integerString != newIntegerString)
                        {
                            try
                            {
                                var newInteger = parser(newIntegerString);
                                var integerBytes = Serializer.Serialize(newInteger);
                                newByteArray = new byte[arrayLength];

                                if (arrayLength >= intSize)
                                {
                                    Array.Copy(integerBytes, 0, newByteArray, 0, intSize);
                                }
                                else
                                {
                                    // Use smaller bits to be compatible with int32, int16, etc.
                                    Array.Copy(integerBytes, intSize - arrayLength, newByteArray, 0, arrayLength);
                                }
                            }
                            catch (FormatException) { }
                            catch (OverflowException) { }
                        }

                        ++line;
                    }

                    Integer("64 bits Unsigned Integer", sizeof(ulong), ulong.Parse);
                    Integer("32 bits Unsigned Integer", sizeof(uint), uint.Parse);
                    Integer("16 bits Unsigned Integer", sizeof(ushort), ushort.Parse);
                    Integer("8 bits Unsigned Integer", sizeof(byte), byte.Parse);
                }
            }

            --EditorGUI.indentLevel;

            EditorGUI.EndProperty();

            // Apply new byte array if need
            if (newByteArray == null)
            {
                return;
            }

            {
                for (var i = 0; i < arrayLength; ++i)
                {
                    if (byteArrayObject.arraySize <= i)
                    {
                        byteArrayObject.InsertArrayElementAtIndex(i);
                    }

                    var element = byteArrayObject.GetArrayElementAtIndex(i);
                    element.intValue = newByteArray[i];
                }

                for (var i = arrayLength; i < byteArrayObject.arraySize; ++i)
                {
                    byteArrayObject.DeleteArrayElementAtIndex(arrayLength);
                }
            }
        }

        public override float GetPropertyHeight(SerializedProperty property, GUIContent label)
        {
            int lineNum = _isFoldout ? 8 : 1;
            return (_lineHeight + _linePadding) * lineNum;
        }

        /// <summary>
        /// Get GUI position
        /// </summary>
        /// <param name="baseRect"></param>
        /// <param name="line">line index</param>
        /// <returns></returns>
        private static Rect GetRect(Rect baseRect, int line)
        {
            return GetSeparatedRect(baseRect, line, 1, 0);
        }

        /// <summary>
        /// Get GUI position
        /// </summary>
        /// <param name="baseRect"></param>
        /// <param name="line">line index</param>
        /// <param name="separation"></param>
        /// <param name="raw"></param>
        /// <param name="rawCount"></param>
        /// <param name="lineCount"></param>
        /// <returns></returns>
        private static Rect GetSeparatedRect(Rect baseRect, int line, int separation, int raw, int rawCount = 1,
            int lineCount = 1)
        {
            var targetRect = baseRect;
            targetRect.x += baseRect.width / separation * raw;
            targetRect.y += (_lineHeight + _linePadding) * line;
            targetRect.width = targetRect.width / separation * rawCount;
            targetRect.height = _lineHeight * lineCount + _linePadding * (lineCount - 1);
            ;
            return targetRect;
        }
    }
#endif
}