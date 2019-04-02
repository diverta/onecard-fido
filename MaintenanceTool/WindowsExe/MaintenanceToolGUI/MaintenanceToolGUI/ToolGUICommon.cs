namespace MaintenanceToolGUI
{
    static class ToolGUICommon
    {
        // macOS版と共通のメッセージ文言を使用
        // 共通
        public const string MSG_INVALID_FIELD = "入力値が不正です。";
        public const string MSG_INVALID_FILE_PATH = "ファイルが存在しません。";
        public const string MSG_INVALID_NUMBER = "入力値が数値ではありません。";
        public const string MSG_SUCCESS = "成功";
        public const string MSG_FAILURE = "失敗";

        // ホーム画面
        public const string MSG_PROMPT_SELECT_PKEY_PATH = "秘密鍵ファイル(PEM)のパスを選択してください";
        public const string MSG_PROMPT_SELECT_CRT_PATH = "証明書ファイル(CRT)のパスを選択してください";
        public const string MSG_ERASE_SKEY_CERT = "FIDO認証器から鍵・証明書・キーハンドルをすべて削除します。";
        public const string MSG_PROMPT_ERASE_SKEY_CERT = "削除後はFIDO認証器によるユーザー登録／ログインができなくなります。\n削除処理を実行しますか？";
        public const string MSG_FORMAT_START_MESSAGE = "{0}を開始します。";
        public const string MSG_FORMAT_END_MESSAGE = "{0}が{1}しました。";

        // コマンド種別に対応する処理名称
        public const string PROCESS_NAME_ERASE_SKEY_CERT = "鍵・証明書・キーハンドル削除処理";
        public const string PROCESS_NAME_INSTALL_SKEY_CERT = "鍵・証明書インストール";
        public const string PROCESS_NAME_HEALTHCHECK = "ヘルスチェック";
        public const string PROCESS_NAME_PAIRING = "ペアリング";
        public const string PROCESS_NAME_TEST_CTAPHID_INIT = "CTAPHID_INITのテスト";

        // ファイル選択／保存ダイアログ用フィルター
        public const string FILTER_SELECT_PEM_PATH = "秘密鍵ファイル (*.pem)|*.pem";
        public const string FILTER_SELECT_CRT_PATH = "証明書ファイル (*.crt)|*.crt";
    }
}
