using System;
using System.IO;
using System.Linq;
using System.Security.Cryptography;
using System.Text;

namespace ToolGUICommon
{
    public class AppLogUtil
    {
        private static AppLogUtil Instance = new AppLogUtil();
        private string ApplicationName;

        private AppLogUtil()
        {
        }

        //
        // 公開用メソッド
        //
        public static void SetApplicationName(string applicationName)
        {
            Instance.ApplicationName = applicationName;
        }

        public static string GetApplicationName()
        {
            return Instance.ApplicationName;
        }

        public static void OutputLogText(string logText, string fname)
        {
            try {
                // ログファイルにメッセージを出力する
                StreamWriter sr = new StreamWriter(new FileStream(fname, FileMode.Append), Encoding.Default);
                sr.WriteLine(logText);
                sr.Close();

            } catch (Exception e) {
                Console.Write(e.Message);
            }
        }
    }

    public static class AppUtil
    {
        // Windows版固有のメッセージ文言
        // USB管理
        public const string MSG_USB_DETECT_FAILED = "USBデバイス検知の開始に失敗しました。";
        public const string MSG_USB_DETECT_STARTED = "USBデバイス検知を開始しました。";
        public const string MSG_USB_DETECT_END = "USBデバイス検知を終了しました。";
        public const string MSG_HID_REMOVED = "USB HIDデバイスが取り外されました。";
        public const string MSG_HID_CONNECTED = "USB HIDデバイスに接続されました。";
        public const string MSG_HID_RESPONSE_RECEIVED = "USB HIDデバイスからレスポンスを受信しました。";
        public const string MSG_HID_REQUEST_SENT = "USB HIDデバイスにリクエストを送信しました。";
        public const string MSG_HID_CMD_RESPONSE_TIMEOUT = "認証器からの応答が受信できませんでした。";
        public const string MSG_FORMAT_NOT_INSTALLED = "{0}が導入されていません。";
        public const string MSG_FORMAT_PROCESS_STARTED = "{0}を開始しました: {1} {2}";
        public const string MSG_FORMAT_PROCESS_EXITED = "{0}が{1}しました: {2} {3}";

        public static void SetOutputLogApplName(string applName)
        {
            // ログ出力を行うアプリケーション名を設定
            AppLogUtil.SetApplicationName(applName);
        }

        public static void OutputLogText(string logText)
        {
            // ログファイルにメッセージを出力する
            string fname = OutputLogFilePath();
            AppLogUtil.OutputLogText(logText, fname);
        }

        private static string OutputLogFilePath()
        {
            // ファイル名を連結して戻す
            return string.Format("{0}\\{1}.log", OutputLogFileDirectoryPath(), AppLogUtil.GetApplicationName());
        }

        public static string OutputLogFileDirectoryPath()
        {
            try {
                // ホームディレクトリー配下に生成
                string dir = string.Format("{0}\\Diverta\\FIDO",
                    Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData));

                // ディレクトリー存在チェック
                if (Directory.Exists(dir) == false) {
                    // ディレクトリーが存在しない場合は新規生成
                    DirectoryInfo dirInfo = Directory.CreateDirectory(dir);
                    Console.Write(string.Format("outputLogText: Directory created at {0}", dir));
                }

                // ディレクトリーを戻す
                return dir;

            } catch (Exception e) {
                Console.Write(e.Message);
                return ".";
            }
        }

        public static void OutputLogError(string message)
        {
            // メッセージに現在時刻を付加し、ログファイルに出力
            OutputLogText(string.Format("{0} [error] {1}", DateTime.Now.ToString(), message));
        }

        public static void OutputLogWarn(string message)
        {
            // メッセージに現在時刻を付加し、ログファイルに出力
            OutputLogText(string.Format("{0} [warn] {1}", DateTime.Now.ToString(), message));
        }

        public static void OutputLogInfo(string message)
        {
            // メッセージに現在時刻を付加し、ログファイルに出力
            OutputLogText(string.Format("{0} [info] {1}", DateTime.Now.ToString(), message));
        }

        public static void OutputLogDebug(string message)
        {
            // メッセージに現在時刻を付加し、ログファイルに出力
            OutputLogText(string.Format("{0} [debug] {1}", DateTime.Now.ToString(), message));
        }

        public static string DumpMessage(byte[] message, int length)
        {
            StringBuilder sb = new StringBuilder();
            for (int i = 0; i < length; i++) {
                sb.Append(string.Format("{0:x2} ", message[i]));
                if ((i % 16 == 15) && (i < length - 1)) {
                    sb.Append("\r\n");
                }
            }
            return sb.ToString();
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

        public static byte[] AES256CBCEncrypt(byte[] key, byte[] data)
        {
            // AES256-CBCにより暗号化
            //   鍵の長さ: 256（32バイト）
            //   ブロックサイズ: 128（16バイト）
            //   暗号利用モード: CBC
            //   初期化ベクター: 0
            AesManaged aes = new AesManaged {
                KeySize = 256,
                BlockSize = 128,
                Mode = CipherMode.CBC,
                IV = new byte[] { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
                Key = key,
                Padding = PaddingMode.None
            };
            return aes.CreateEncryptor().TransformFinalBlock(data, 0, data.Length);
        }

        public static byte[] AES256CBCDecrypt(byte[] key, byte[] data)
        {
            // AES256-CBCにより復号化
            //   鍵の長さ: 256（32バイト）
            //   ブロックサイズ: 128（16バイト）
            //   暗号利用モード: CBC
            //   初期化ベクター: 0
            AesManaged aes = new AesManaged {
                KeySize = 256,
                BlockSize = 128,
                Mode = CipherMode.CBC,
                IV = new byte[] { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
                Key = key,
                Padding = PaddingMode.None
            };
            return aes.CreateDecryptor().TransformFinalBlock(data, 0, data.Length);
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

        public static int CalculateDecimalVersion(string versionStr)
        {
            // バージョン文字列 "1.2.11" -> "010211" 形式に変換
            int decimalVersion = 0;
            foreach (string element in versionStr.Split('.')) {
                decimalVersion = decimalVersion * 100 + int.Parse(element);
            }
            return decimalVersion;
        }
    }
}
