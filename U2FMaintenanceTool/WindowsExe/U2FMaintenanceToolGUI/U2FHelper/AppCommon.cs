namespace U2FHelper
{
    static class AppCommon
    {
        // macOS版と共通のメッセージ文言を使用
        // 共通
        public const string MSG_INVALID_FILE_PATH = "ファイルが存在しません。";
        public const string MSG_SUCCESS = "成功";
        public const string MSG_FAILURE = "失敗";

        // ホーム画面
        public const string MSG_FORMAT_START_MESSAGE = "{0}を開始します。";
        public const string MSG_FORMAT_END_MESSAGE = "{0}が{1}しました。";

        // コマンド種別に対応する処理名称
        public const string PROCESS_NAME_PAIRING = "ペアリング";

        // Windows版固有のメッセージ文言
        // USB管理
        public const string MSG_U2FCOMMAND_PROCESS = "U2F管理コマンドの実行";
        public const string MSG_HID_BLE_CONNECTION = "U2F HIDデバイス～BLEデバイス間の連携処理";
        public const string MSG_USB_DETECT_FAILED = "USBデバイス検知の開始に失敗しました。";
        public const string MSG_USB_DETECT_STARTED = "USBデバイス検知を開始しました。";
        public const string MSG_USB_DETECT_END = "USBデバイス検知を終了しました。";
        public const string MSG_HID_REMOVED = "U2F HIDデバイスが取り外されました。";
        public const string MSG_HID_CONNECTED = "U2F HIDデバイスに接続されました。";
        public const string MSG_HID_MESSAGE_TRANSFERRED = "U2F HIDデバイスからメッセージが転送されました。";
        public const string MSG_FORMAT_NOT_INSTALLED = "{0}が導入されていません。";
        public const string MSG_FORMAT_PROCESS_STARTED = "{0}を開始しました: {1} {2}";
        public const string MSG_FORMAT_PROCESS_EXITED = "{0}が{1}しました: {2} {3}";

        // ファイル名
        public const string FILENAME_U2FHELPER_LOG = "U2FHelper.log";
        public const string FILENAME_U2FCOMMAND_EXE = "U2FHelper.log";
    }
}
