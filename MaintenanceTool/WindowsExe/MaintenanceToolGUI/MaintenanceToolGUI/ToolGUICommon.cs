namespace MaintenanceToolGUI
{
    static class ToolGUICommon
    {
        // macOS版と共通のメッセージ文言を使用
        // 共通
        public const string MSG_INVALID_FIELD = "入力値が不正です。";
        public const string MSG_INVALID_FIELD_SIZE = "入力値の長さが不正です。";
        public const string MSG_INVALID_FILE_PATH = "ファイルが存在しません。";
        public const string MSG_INVALID_NUMBER = "入力値が数字ではありません。";
        public const string MSG_SUCCESS = "成功";
        public const string MSG_FAILURE = "失敗";

        // ホーム画面
        public const string MSG_PROMPT_SELECT_PKEY_PATH = "秘密鍵ファイル(PEM)のパスを選択してください";
        public const string MSG_PROMPT_SELECT_CRT_PATH = "証明書ファイル(CRT)のパスを選択してください";
        public const string MSG_ERASE_SKEY_CERT = "FIDO認証器から鍵・証明書・キーハンドルをすべて削除します。";
        public const string MSG_PROMPT_ERASE_SKEY_CERT = "削除後はFIDO認証器によるユーザー登録／ログインができなくなります。\n削除処理を実行しますか？";
        public const string MSG_FORMAT_START_MESSAGE = "{0}を開始します。";
        public const string MSG_FORMAT_END_MESSAGE = "{0}が{1}しました。";

        // PIN設定画面
        public const string MSG_PROMPT_INPUT_NEW_PIN = "新しいPINコードを４〜16桁で入力してください";
        public const string MSG_PROMPT_INPUT_NEW_PIN_CONFIRM = "新しいPINコード（確認用）を４〜16桁で入力してください";
        public const string MSG_PROMPT_INPUT_OLD_PIN = "変更前のPINコードを４〜16桁で入力してください";
        public const string MSG_PROMPT_INPUT_NEW_PIN_NUM = "新しいPINコードを数字で入力してください";
        public const string MSG_PROMPT_INPUT_NEW_PIN_CONF_NUM = "新しいPINコード（確認用）を数字で入力してください";
        public const string MSG_PROMPT_INPUT_OLD_PIN_NUM = "変更前のPINコードを数字で入力してください";
        public const string MSG_PROMPT_INPUT_PIN_CONFIRM_CRCT = "確認用のPINコードを正しく入力してください";

        public const string MSG_CLEAR_PIN_CODE = "FIDO認証器に設定されたPINコードを解除します。";
        public const string MSG_PROMPT_CLEAR_PIN_CODE = "解除後はFIDO認証器によるログインができなくなります。\n（インストールされた鍵・証明書はそのまま残ります）\n\nPINコード解除処理を実行しますか？";
        public const string MSG_CLEAR_PIN_CODE_COMMENT1 = "  ユーザー確認が必要となりますので、";
        public const string MSG_CLEAR_PIN_CODE_COMMENT2 = "  FIDO認証器上のユーザー確認LEDが高速点滅したら、";
        public const string MSG_CLEAR_PIN_CODE_COMMENT3 = "  MAIN SWを１回押してください.";

        // コマンド種別に対応する処理名称
        public const string PROCESS_NAME_ERASE_SKEY_CERT = "鍵・証明書・キーハンドル削除処理";
        public const string PROCESS_NAME_INSTALL_SKEY_CERT = "鍵・証明書インストール";
        public const string PROCESS_NAME_BLE_CTAP2_HEALTHCHECK = "BLE CTAP2ヘルスチェック";
        public const string PROCESS_NAME_BLE_U2F_HEALTHCHECK = "BLE U2Fヘルスチェック";
        public const string PROCESS_NAME_TEST_BLE_PING = "BLE PINGテスト";
        public const string PROCESS_NAME_PAIRING = "ペアリング";
        public const string PROCESS_NAME_HID_CTAP2_HEALTHCHECK = "HID CTAP2ヘルスチェック";
        public const string PROCESS_NAME_HID_U2F_HEALTHCHECK = "HID U2Fヘルスチェック";
        public const string PROCESS_NAME_TEST_CTAPHID_PING = "HID PINGテスト";
        public const string PROCESS_NAME_GET_FLASH_STAT = "Flash ROM情報取得";
        public const string PROCESS_NAME_CLIENT_PIN_SET = "PINコード新規設定";
        public const string PROCESS_NAME_CLIENT_PIN_CHANGE = "PINコード変更";
        public const string PROCESS_NAME_AUTH_RESET = "PINコード解除";

        // ファイル選択／保存ダイアログ用フィルター
        public const string FILTER_SELECT_PEM_PATH = "秘密鍵ファイル (*.pem)|*.pem";
        public const string FILTER_SELECT_CRT_PATH = "証明書ファイル (*.crt)|*.crt";

        // PINコードの最小／最大桁数
        public const int PIN_CODE_SIZE_MIN = 4;
        public const int PIN_CODE_SIZE_MAX = 16;
    }
}
