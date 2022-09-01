﻿namespace MaintenanceToolApp
{
    public static class AppCommon
    {
        // 起動時のメッセージ文言
        public const string MSG_TOOL_TITLE = "FIDO認証器管理ツール";
        public const string MSG_INVALID_USER_ROLL = "このツールは、管理者として実行してください。\n\nプログラムアイコンを右クリックして、\nメニューから「管理者として実行」を選択します。";
        public const string MSG_ERROR_DOUBLE_START = "既に起動されています。";
        public const string MSG_CMDTST_PROMPT_USB_PORT_SET = "FIDO認証器をUSBポートに装着してから実行してください。";
        public const string MSG_CMDTST_MENU_NOT_SUPPORTED = "このメニューは実行できません。";

        // メッセージリソース
        public const string MSG_USB_DETECT_FAILED = "USBデバイス検知の開始に失敗しました。";
        public const string MSG_USB_DETECT_STARTED = "USBデバイス検知を開始しました。";
        public const string MSG_USB_DETECT_END = "USBデバイス検知を終了しました。";
        public const string MSG_HID_REMOVED = "USB HIDデバイスが取り外されました。";
        public const string MSG_HID_CONNECTED = "USB HIDデバイスに接続されました。";

        // コマンド種別に対応する処理名称
        public const string PROCESS_NAME_BLE_CTAP2_HEALTHCHECK = "BLE CTAP2ヘルスチェック";
        public const string PROCESS_NAME_BLE_U2F_HEALTHCHECK = "BLE U2Fヘルスチェック";
        public const string PROCESS_NAME_TEST_BLE_PING = "BLE PINGテスト";
        public const string PROCESS_NAME_HID_CTAP2_HEALTHCHECK = "HID CTAP2ヘルスチェック";
        public const string PROCESS_NAME_HID_U2F_HEALTHCHECK = "HID U2Fヘルスチェック";
        public const string PROCESS_NAME_TEST_CTAPHID_PING = "HID PINGテスト";
        public const string PROCESS_NAME_GET_FLASH_STAT = "Flash ROM情報取得";
        public const string PROCESS_NAME_GET_VERSION_INFO = "ファームウェアバージョン情報取得";
        public const string PROCESS_NAME_TOOL_VERSION_INFO = "管理ツールのバージョンを参照";
        public const string PROCESS_NAME_VIEW_LOG_FILE = "管理ツールのログを参照";
        public const string PROCESS_NAME_NONE = "";

        // ホーム画面
        public const string MSG_FORMAT_START_MESSAGE = "{0}を開始します。";
        public const string MSG_FORMAT_END_MESSAGE = "{0}が{1}しました。";
        public const string MSG_SUCCESS = "成功";
        public const string MSG_FAILURE = "失敗";
        public const string MSG_OCCUR_UNKNOWN_ERROR = "不明なエラーが発生しました。";

        // ヘルスチェック機能
        public const string MSG_PROMPT_INPUT_HCHECK_PIN = "PINコードを４〜16桁で入力してください";
        public const string MSG_PROMPT_INPUT_HCHECK_PIN_NUM = "PINコードを数字で入力してください";
        public const string MSG_CMDTST_INVALID_NONCE = "CTAPHID_INITコマンドが失敗しました。";
        public const string MSG_CMDTST_INVALID_PING = "CTAPHID_PINGコマンドが失敗しました。";

        public const string MSG_OCCUR_KEYHANDLE_ERROR = "キーハンドルが存在しません。再度ユーザー登録を実行してください。";
        public const string MSG_OCCUR_SKEYNOEXIST_ERROR = "鍵・証明書がインストールされていません。鍵・証明書インストール処理を実行してください。";
        public const string MSG_OCCUR_PAIRINGMODE_ERROR = "ペアリングモードでは、ペアリング実行以外の機能は使用できません。\r\nペアリングモードを解除してから、機能を再度実行してください。";

        // ユーティリティー機能
        public const string MSG_LABEL_NAME_TOOL_VERSION_INFO = "管理ツールのバージョン";
        public const string MSG_FORMAT_UTILITY_VIEW_LOG_FILE_ERR = "管理ツールのログファイル格納フォルダーを参照できませんでした。{0}";

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
        public const string MSG_VERSION_INFO_SECURE_IC_AVAIL = "  セキュアIC: 搭載";
        public const string MSG_VERSION_INFO_SECURE_IC_UNAVAIL = "  セキュアIC: 非搭載";

        // Windows版固有のメッセージ文言
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
    }
}
