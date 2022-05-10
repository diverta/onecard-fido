namespace DevelopmentToolGUI
{
    public static class AppCommon
    {
        // macOS版と共通のメッセージ文言を使用
        // 共通
        public const string MSG_SUCCESS = "成功";
        public const string MSG_FAILURE = "失敗";

        // ホーム画面
        public const string MSG_PROMPT_SELECT_PKEY_PATH = "秘密鍵ファイル(PEM)のパスを選択してください";
        public const string MSG_PROMPT_SELECT_CRT_PATH = "証明書ファイル(CRT)のパスを選択してください";
        public const string MSG_ERASE_SKEY_CERT = "FIDO認証器から鍵・証明書・ユーザー登録情報をすべて削除します。";
        public const string MSG_PROMPT_ERASE_SKEY_CERT = "削除後はFIDO認証器によるユーザー登録／ログインができなくなります。\n削除処理を実行しますか？";
        public const string MSG_INSTALL_SKEY_CERT = "FIDO認証器に鍵・証明書をインストールします。";
        public const string MSG_PROMPT_INSTL_SKEY_CERT = "インストールを実行しますか？";
        public const string MSG_FORMAT_START_MESSAGE = "{0}を開始します。";
        public const string MSG_FORMAT_END_MESSAGE = "{0}が{1}しました。";
        public const string MSG_OCCUR_UNKNOWN_ERROR = "不明なエラーが発生しました。";

        // コマンド種別に対応する処理名称
        public const string PROCESS_NAME_ERASE_SKEY_CERT = "鍵・証明書の削除";
        public const string PROCESS_NAME_INSTALL_SKEY_CERT = "鍵・証明書インストール";

        // ファイル選択／保存ダイアログ用フィルター
        public const string FILTER_SELECT_PEM_PATH = "秘密鍵ファイル (*.pem)|*.pem";
        public const string FILTER_SELECT_CRT_PATH = "証明書ファイル (*.crt)|*.crt";

        // 起動時のメッセージ文言
        public const string MSG_INVALID_USER_ROLL = "このツールは、管理者として実行してください。\n\nプログラムアイコンを右クリックして、\nメニューから「管理者として実行」を選択します。";
        public const string MSG_ERROR_DOUBLE_START = "既に起動されています。";

        //
        // CTAP2関連共通リソース
        //
        // CBORサブコマンドバイトに関する定義
        public const byte CTAP2_CBORCMD_NONE = 0x00;
        public const byte CTAP2_CBORCMD_CLIENT_PIN = 0x06;
        public const byte CTAP2_SUBCMD_CLIENT_PIN_GET_AGREEMENT = 0x02;

        // トランスポート種別
        public const byte TRANSPORT_NONE = 0x00;
        public const byte TRANSPORT_BLE = 0x01;
        public const byte TRANSPORT_HID = 0x02;

        // 鍵・証明書インストール関連
        public const string MSG_CANNOT_RECV_DEVICE_PUBLIC_KEY = "公開鍵を認証器から受け取ることができませんでした。";
        public const string MSG_CANNOT_READ_SKEY_PEM_FILE = "鍵ファイルを読み込むことができません。";
        public const string MSG_CANNOT_READ_CERT_CRT_FILE = "証明書ファイルを読み込むことができません。";
        public const string MSG_CANNOT_CRYPTO_SKEY_CERT_DATA = "鍵・証明書の転送データを暗号化できませんでした。";
        public const string MSG_INVALID_SKEY_OR_CERT = "秘密鍵または公開鍵の内容が不正です。";

        // コマンドテスト関連メッセージ
        public const string MSG_CMDTST_INVALID_NONCE = "CTAPHID_INITコマンドが失敗しました。";
        public const string MSG_CMDTST_PROMPT_USB_PORT_SET = "FIDO認証器をUSBポートに装着してから実行してください。";

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
        };
    }
}
