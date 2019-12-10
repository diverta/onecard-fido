using System;
using System.IO;
using System.Linq;
using System.Security.Cryptography;
using System.Text;

namespace MaintenanceToolCommon
{
    public static class AppCommon
    {
        //
        // CTAP2関連共通リソース
        //
        // CBORサブコマンドバイトに関する定義
        public const byte CTAP2_CBORCMD_NONE = 0x00;
        public const byte CTAP2_CBORCMD_MAKE_CREDENTIAL = 0x01;
        public const byte CTAP2_CBORCMD_GET_ASSERTION = 0x02;
        public const byte CTAP2_CBORCMD_CLIENT_PIN = 0x06;
        public const byte CTAP2_CBORCMD_AUTH_RESET = 0x07;
        public const byte CTAP2_SUBCMD_CLIENT_PIN_GET_AGREEMENT = 0x02;
        public const byte CTAP2_SUBCMD_CLIENT_PIN_SET = 0x03;
        public const byte CTAP2_SUBCMD_CLIENT_PIN_CHANGE = 0x04;
        public const byte CTAP2_SUBCMD_CLIENT_PIN_GET_PIN_TOKEN = 0x05;

        // トランスポート種別
        public const byte TRANSPORT_NONE = 0x00;
        public const byte TRANSPORT_BLE = 0x01;
        public const byte TRANSPORT_HID = 0x02;

        // macOS版と共通のメッセージ文言を使用
        // 共通
        public const string MSG_INVALID_FILE_PATH = "ファイルが存在しません。";
        public const string MSG_SUCCESS = "成功";
        public const string MSG_FAILURE = "失敗";

        // ホーム画面
        public const string MSG_OCCUR_UNKNOWN_ERROR = "不明なエラーが発生しました。";
        public const string MSG_FORMAT_START_MESSAGE = "{0}を開始します。";
        public const string MSG_FORMAT_END_MESSAGE = "{0}が{1}しました。";
        public const string MSG_OCCUR_KEYHANDLE_ERROR = "キーハンドルが存在しません。再度ユーザー登録を実行してください。";
        public const string MSG_OCCUR_SKEYNOEXIST_ERROR = "鍵・証明書がインストールされていません。鍵・証明書インストール処理を実行してください。";
        public const string MSG_OCCUR_PAIRINGMODE_ERROR = "ペアリングモードでは、ペアリング実行以外の機能は使用できません。\r\nペアリングモードを解除してから、機能を再度実行してください。";

        // 鍵・証明書インストール関連
        public const string MSG_CANNOT_RECV_DEVICE_PUBLIC_KEY = "公開鍵を認証器から受け取ることができませんでした。";
        public const string MSG_CANNOT_READ_SKEY_PEM_FILE = "鍵ファイルを読み込むことができません。";
        public const string MSG_CANNOT_READ_CERT_CRT_FILE = "証明書ファイルを読み込むことができません。";
        public const string MSG_CANNOT_CRYPTO_SKEY_CERT_DATA = "鍵・証明書の転送データを暗号化できませんでした。";

        // コマンド種別に対応する処理名称
        public const string PROCESS_NAME_PAIRING = "ペアリング";

        // ヘルスチェック関連メッセージ
        public const string MSG_HCHK_U2F_REGISTER_SUCCESS = "U2F Registerが成功しました。";
        public const string MSG_HCHK_U2F_AUTHENTICATE_START = "U2F Authenticateを開始します.";
        public const string MSG_HCHK_U2F_AUTHENTICATE_COMMENT1 = "  ユーザー所在確認が必要となりますので、";
        public const string MSG_HCHK_U2F_AUTHENTICATE_COMMENT2 = "  FIDO認証器上のユーザー所在確認LEDが点滅したら、";
        public const string MSG_HCHK_U2F_AUTHENTICATE_COMMENT3 = "  MAIN SWを１回押してください.";
        public const string MSG_HCHK_U2F_AUTHENTICATE_SUCCESS = "U2F Authenticateが成功しました。";

        // コマンドテスト関連メッセージ
        public const string MSG_CMDTST_INVALID_NONCE = "CTAPHID_INITコマンドが失敗しました。";
        public const string MSG_CMDTST_INVALID_PING = "CTAPHID_PINGコマンドが失敗しました。";
        public const string MSG_CMDTST_PROMPT_USB_PORT_SET = "FIDO認証器をUSBポートに装着してから実行してください。";
        public const string MSG_CMDTST_MENU_NOT_SUPPORTED = "このメニューは実行できません。";

        // PINコードチェック関連メッセージ
        public const string MSG_CTAP2_ERR_PIN_INVALID = "入力されたPINコードが違います。正しいPINコードを入力してください。";
        public const string MSG_CTAP2_ERR_PIN_BLOCKED = "使用中のPINコードが無効となりました。新しいPINコードを設定し直してください。";
        public const string MSG_CTAP2_ERR_PIN_AUTH_BLOCKED = "PIN認証が無効となりました。認証器をUSBポートから取り外してください。";
        public const string MSG_CTAP2_ERR_PIN_NOT_SET = "PINコードが認証器に設定されていません。PINコードを新規設定してください。";

        // CTAP2ヘルスチェック関連メッセージ
        public const string MSG_HCHK_CTAP2_LOGIN_TEST_START = "ログインテストを開始します.";
        public const string MSG_HCHK_CTAP2_LOGIN_TEST_COMMENT1 = "  ユーザー所在確認が必要となりますので、";
        public const string MSG_HCHK_CTAP2_LOGIN_TEST_COMMENT2 = "  FIDO認証器上のユーザー所在確認LEDが点滅したら、";
        public const string MSG_HCHK_CTAP2_LOGIN_TEST_COMMENT3 = "  MAIN SWを１回押してください.";

        // Flash ROM情報取得関連メッセージ
        public const string MSG_FSTAT_REMAINING_RATE = "Flash ROMの空き容量は{0:0.0}％です。";
        public const string MSG_FSTAT_NON_REMAINING_RATE = "Flash ROMの空き容量を取得できませんでした。";
        public const string MSG_FSTAT_CORRUPTING_AREA_NOT_EXIST = "破損している領域は存在しません。";
        public const string MSG_FSTAT_CORRUPTING_AREA_EXIST = "破損している領域が存在します。";

        // バージョン情報取得関連メッセージ
        public const string MSG_VERSION_INFO_HEADER = "FIDO認証器のバージョン情報";
        public const string MSG_VERSION_INFO_DEVICE_NAME = "  デバイス名: {0}";
        public const string MSG_VERSION_INFO_FW_REV = "  ファームウェアのバージョン: {0}";
        public const string MSG_VERSION_INFO_HW_REV = "  ハードウェアのバージョン: {0}";

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

        // BLE関連のメッセージ文言
        public const string MSG_BLE_U2F_SERVICE_NOT_FOUND = "FIDO BLEサービスが見つかりません。";
        public const string MSG_BLE_U2F_SERVICE_FOUND = "FIDO BLEサービスが見つかりました。";
        public const string MSG_U2F_DEVICE_CONNECT_FAILED = "FIDO認証器の接続に失敗しました。";
        public const string MSG_U2F_DEVICE_CONNECTED = "FIDO認証器に接続しました。";
        public const string MSG_U2F_DEVICE_DISCONNECTED = "FIDO認証器の接続が切断されました。";
        public const string MSG_BLE_CHARACT_NOT_DISCOVERED = "FIDO BLEサービスと通信できません。";
        public const string MSG_BLE_NOTIFICATION_FAILED = "FIDO BLEサービスからデータを受信できません。";
        public const string MSG_BLE_NOTIFICATION_START = "受信データの監視を開始します。";
        public const string MSG_REQUEST_SEND_FAILED = "リクエスト送信が失敗しました。";
        public const string MSG_REQUEST_SENT = "リクエストを送信しました。";
        public const string MSG_RESPONSE_RECEIVED = "レスポンスを受信しました。";
        public const string MSG_BLE_INVALID_PING = "BLE経由のPINGコマンドが失敗しました。";

        // BLEペアリング関連のメッセージ文言
        public const string MSG_BLE_PARING_ERR_BT_OFF = "Bluetoothがオフになっています。Bluetoothをオンにしてください。";
        public const string MSG_BLE_PARING_ERR_TIMED_OUT = "FIDO認証器が停止している可能性があります。FIDO認証器の電源を入れ、PCのUSBポートから外してください。";
        public const string MSG_BLE_PARING_ERR_PAIR_MODE = "FIDO認証器がペアリングモードでない可能性があります。FIDO認証器のMAIN SWを３秒間以上長押して、ペアリングモードに遷移させてください。";
        public const string MSG_BLE_PARING_ERR_UNKNOWN = "FIDO認証器とのペアリング時に不明なエラーが発生しました。";

        // BLE接続無効化時のメッセージ文言
        public const string MSG_BLE_ERR_CONN_DISABLED = "BLE接続が無効となりました。";
        public const string MSG_BLE_ERR_CONN_DISABLED_SUB1 = "大変お手数をお掛けしますが、管理ツールを終了後、再度起動させてください。";

        public static void OutputLogText(string logText)
        {
            try {
                // ログファイルにメッセージを出力する
                string fname = OutputLogFilePath();
                StreamWriter sr = new StreamWriter(
                    (new FileStream(fname, FileMode.Append)), Encoding.Default);
                sr.WriteLine(logText);
                sr.Close();

            } catch (Exception e) {
                Console.Write(e.Message);
            }
        }

        private static string OutputLogFilePath()
        {
            string fileName = "MaintenanceTool.log";
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

                // ファイル名を連結して戻す
                return string.Format("{0}\\{1}", dir, fileName);

            } catch (Exception e) {
                Console.Write(e.Message);
                return fileName;
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

        private static byte[] GetSubArray(byte[] src, int startIndex, int count)
        {
            byte[] dst = new byte[count];
            Array.Copy(src, startIndex, dst, 0, count);
            return dst;
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

        //
        // 処理区分
        //
        public enum RequestType
        {
            None = 0,
            //
            // メンテナンス機能
            //
            EraseSkeyCert,
            InstallSkeyCert,
            ToolPreferenceCommand,
            //
            // U2F
            //
            TestRegister,
            TestAuthenticateCheck,
            TestAuthenticate,
            //
            // CTAP2
            //
            ClientPinSet,
            TestCtapHidPing,
            TestMakeCredential,
            TestGetAssertion,
            AuthReset
        };
    }
}
