using System;
using System.Linq;
using System.Reflection;
using System.Text.RegularExpressions;

namespace ToolAppCommon
{
    public class AppUtil
    {
        private static readonly AppUtil Instance = new AppUtil();
        private readonly string AppVersion;
        private readonly string AppCopyright;
        private readonly string AppBundleName;

        private AppUtil()
        {
            // ツールのバージョン、著作権情報を取得
            AppVersion = string.Format("Version {0}", GetAppVersion());
            AppCopyright = GetAppCopyright();
            AppBundleName = GetAppBundleName();
        }

        private static string GetAppVersion()
        {
            // 製品バージョン文字列を戻す
            Assembly asm = Assembly.GetExecutingAssembly();
            System.Diagnostics.FileVersionInfo ver = System.Diagnostics.FileVersionInfo.GetVersionInfo(asm.Location);
            string? versionString = ver.ProductVersion;
            if (versionString == null) {
                return string.Empty;
            } else {
                return versionString;
            }
        }

        private static string GetAppCopyright()
        {
            // 著作権情報を戻す
            Assembly asm = Assembly.GetExecutingAssembly();
            Attribute? attribute = Attribute.GetCustomAttribute(asm, typeof(AssemblyCopyrightAttribute));
            if (attribute == null) {
                return string.Empty;
            }
            AssemblyCopyrightAttribute copyright = (AssemblyCopyrightAttribute)attribute;
            return copyright.Copyright;
        }

        private static string GetAppBundleName()
        {
            // アプリケーションバンドル文字列を戻す
            AssemblyName assemblyName = Assembly.GetExecutingAssembly().GetName();
            if (assemblyName.Name == null) {
                return string.Empty;
            } else {
                return assemblyName.Name;
            }
        }

        //
        // 公開用メソッド
        //
        public static string GetAppVersionString()
        {
            return Instance.AppVersion;
        }

        public static string GetAppCopyrightString()
        {
            return Instance.AppCopyright;
        }

        public static string GetAppBundleNameString()
        {
            return Instance.AppBundleName;
        }

        // 
        // ユーティリティー
        //
        public static int CalculateDecimalVersion(string versionStr)
        {
            // バージョン文字列 "1.2.11" -> "010211" 形式に変換
            int decimalVersion = 0;
            foreach (string element in versionStr.Split('.')) {
                decimalVersion = decimalVersion * 100 + int.Parse(element);
            }
            return decimalVersion;
        }

        public static byte[] ExtractCBORBytesFromResponse(byte[] message, int length)
        {
            // レスポンスされたCBORを抽出
            //   CBORバイト配列はレスポンスの２バイト目以降
            int cborLength = length - 1;
            byte[] cborBytes = new byte[cborLength];
            for (int i = 0; i < cborLength; i++) {
                cborBytes[i] = message[1 + i];
            }
            return cborBytes;
        }

        public static bool CompareBytes(byte[] src, byte[] dest, int size)
        {
            for (int i = 0; i < size; i++) {
                if (src[i] != dest[i]) {
                    return false;
                }
            }
            return true;
        }

        public static int ToInt32(byte[] value, int startIndex, bool changeEndian = false)
        {
            byte[] sub = GetSubArray(value, startIndex, 4);
            if (changeEndian == true) {
                sub = sub.Reverse().ToArray();
            }
            return BitConverter.ToInt32(sub, 0);
        }

        public static int ToInt16(byte[] value, int startIndex, bool changeEndian = false)
        {
            byte[] sub = GetSubArray(value, startIndex, 2);
            if (changeEndian == true) {
                sub = sub.Reverse().ToArray();
            }
            return BitConverter.ToInt16(sub, 0);
        }

        public static UInt16 ToUInt16(byte[] value, int startIndex, bool changeEndian = false)
        {
            byte[] sub = GetSubArray(value, startIndex, 2);
            if (changeEndian == true) {
                sub = sub.Reverse().ToArray();
            }
            return BitConverter.ToUInt16(sub, 0);
        }

        private static byte[] GetSubArray(byte[] src, int startIndex, int count)
        {
            byte[] dst = new byte[count];
            Array.Copy(src, startIndex, dst, 0, count);
            return dst;
        }

        public static void ConvertUint32ToLEBytes(UInt32 ui, byte[] b, int offset)
        {
            byte[] s = BitConverter.GetBytes(ui);
            for (int i = 0; i < s.Length; i++) {
                b[i + offset] = s[i];
            }
        }

        public static void ConvertUint16ToLEBytes(UInt16 ui, byte[] b, int offset)
        {
            byte[] s = BitConverter.GetBytes(ui);
            for (int i = 0; i < s.Length; i++) {
                b[i + offset] = s[i];
            }
        }

        public static void ConvertUint32ToBEBytes(UInt32 ui, byte[] b, int offset)
        {
            byte[] s = BitConverter.GetBytes(ui);
            for (int i = 0; i < s.Length; i++) {
                b[i + offset] = s[s.Length - 1 - i];
            }
        }

        public static void ConvertUint16ToBEBytes(UInt16 ui, byte[] b, int offset)
        {
            byte[] s = BitConverter.GetBytes(ui);
            for (int i = 0; i < s.Length; i++) {
                b[i + offset] = s[s.Length - 1 - i];
            }
        }

        public static string ReplaceCRLF(string src)
        {
            return new Regex("\r\n|\n").Replace(src, "");
        }
    }
}
