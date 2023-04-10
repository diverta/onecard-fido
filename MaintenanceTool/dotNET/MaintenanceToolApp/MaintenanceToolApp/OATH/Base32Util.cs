using System;

namespace MaintenanceTool.OATH
{
    internal class Base32Util
    {
        public static bool Decode(string input, out byte[] returnArray)
        {
            returnArray = Array.Empty<byte>();
            if (string.IsNullOrEmpty(input)) {
                return false;
            }

            // Remove padding characters
            input = input.TrimEnd('=');
            
            // This must be TRUNCATED
            int byteCount = input.Length * 5 / 8;
            returnArray = new byte[byteCount];

            byte curByte = 0;
            byte bitsRemaining = 8;
            int mask;
            int arrayIndex = 0;

            foreach (char c in input) {
                int cValue;
                if (CharToValue(c, out cValue) == false) {
                    return false;
                }

                if (bitsRemaining > 5) {
                    mask = cValue << (bitsRemaining - 5);
                    curByte = (byte)(curByte | mask);
                    bitsRemaining -= 5;
                } else {
                    mask = cValue >> (5 - bitsRemaining);
                    curByte = (byte)(curByte | mask);
                    returnArray[arrayIndex++] = curByte;
                    curByte = (byte)(cValue << (3 + bitsRemaining));
                    bitsRemaining += 3;
                }
            }

            // If we didn't end with a full byte
            if (arrayIndex != byteCount) {
                returnArray[arrayIndex] = curByte;
            }

            return true;
        }

        public static bool Encode(byte[] input, out string encoded)
        {
            encoded = string.Empty;
            if (input == null || input.Length == 0) {
                return false;
            }

            int charCount = (int)Math.Ceiling(input.Length / 5d) * 8;
            char[] returnArray = new char[charCount];

            char c;
            byte nextChar = 0;
            byte bitsRemaining = 5;
            int arrayIndex = 0;

            foreach (byte b in input) {
                nextChar = (byte)(nextChar | (b >> (8 - bitsRemaining)));
                if (ValueToChar(nextChar, out c) == false) {
                    return false;
                }
                returnArray[arrayIndex++] = c;

                if (bitsRemaining < 4) {
                    nextChar = (byte)((b >> (3 - bitsRemaining)) & 31);
                    if (ValueToChar(nextChar, out c) == false) {
                        return false;
                    }
                    returnArray[arrayIndex++] = c;
                    bitsRemaining += 5;
                }

                bitsRemaining -= 3;
                nextChar = (byte)((b << bitsRemaining) & 31);
            }

            // If we didn't end with a full char, padding with '='
            if (arrayIndex != charCount) {
                if (ValueToChar(nextChar, out c) == false) {
                    return false;
                }
                returnArray[arrayIndex++] = c;
                while (arrayIndex != charCount) returnArray[arrayIndex++] = '=';
            }

            encoded = new string(returnArray);
            return true;
        }

        private static bool CharToValue(char c, out int cValue)
        {
            int value = (int)c;
            cValue = value;

            // 65-90 == Uppercase letters
            if (value < 91 && value > 64) {
                cValue = value - 65;
                return true;
            }

            // 50-55 == Numbers 2-7
            if (value < 56 && value > 49) {
                cValue = value - 24;
                return true;
            }

            // 97-122 == Lowercase letters
            if (value < 123 && value > 96) {
                cValue = value - 97;
                return true;
            }

            // Character is not a Base32 character.
            return false;
        }

        private static bool ValueToChar(byte b, out char cValue)
        {
            cValue = (char)b;
            if (b < 26) {
                cValue = (char)(b + 65);
                return true;
            }

            if (b < 32) {
                cValue = (char)(b + 24);
                return true;
            }

            // Byte is not a value Base32 value.
            return false;
        }
    }
}
