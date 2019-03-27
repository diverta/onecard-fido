namespace U2FMaintenanceToolGUI
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
        public const string MSG_ERASE_SKEY_CERT = "One Cardから鍵・証明書・キーハンドルをすべて削除します。";
        public const string MSG_PROMPT_ERASE_SKEY_CERT = "削除後はOne CardによるU2F認証ができなくなります。\n削除処理を実行しますか？";
        public const string MSG_FORMAT_START_MESSAGE = "{0}を開始します。";
        public const string MSG_FORMAT_END_MESSAGE = "{0}が{1}しました。";
        // 証明書要求ファイル作成画面
        public const string MSG_PROMPT_SELECT_PEM_PATH = "使用する秘密鍵ファイル(PEM)を選択してください。";
        public const string MSG_PROMPT_CREATE_CSR_PATH = "作成する証明書要求ファイル(CSR)名を指定してください。";
        public const string MSG_PROMPT_CREATE_PEM_PATH = "作成する秘密鍵ファイル(PEM)名を指定してください。";
        public const string MSG_PROMPT_INPUT_CN = "実際に接続されるURLのFQDN（例：www.diverta.co.jp）を入力してください。";
        public const string MSG_PROMPT_INPUT_O = "申請組織の名称（例：Diverta Inc.）を入力してください。";
        public const string MSG_PROMPT_INPUT_L = "申請組織の事業所住所の市区町村名（例：Shinjuku-ku、Yokohama-shi等）を入力してください。";
        public const string MSG_PROMPT_INPUT_ST = "申請組織の事業所住所の都道府県名（例：Tokyo、Kanagawa）を入力してください。";
        public const string MSG_PROMPT_INPUT_C = "申請組織の事業所住所の国名（例：JP）を入力してください。";
        public const string MSG_PROMPT_EXIST_PEM_PATH = "実在する秘密鍵ファイル(PEM)のパスを指定してください。";

        // 自己署名証明書ファイル作成画面
        public const string MSG_PROMPT_SELECT_CSR_PATH = "使用する証明書要求ファイル(CSR)を選択してください。";
        public const string MSG_PROMPT_CREATE_CRT_PATH = "作成する自己署名証明書ファイル(CRT)名を指定してください。";
        public const string MSG_PROMPT_EXIST_CSR_PATH = "実在する証明書要求ファイル(CSR)のパスを指定してください。";
        public const string MSG_PROMPT_INPUT_CRT_DAYS = "自己署名証明書の有効期間（日数）を数値で入力してください。";

        // コマンド種別に対応する処理名称
        public const string PROCESS_NAME_ERASE_SKEY_CERT = "鍵・証明書・キーハンドル削除処理";
        public const string PROCESS_NAME_INSTALL_SKEY_CERT = "鍵・証明書インストール";
        public const string PROCESS_NAME_HEALTHCHECK = "ヘルスチェック";
        public const string PROCESS_NAME_CREATE_KEYPAIR_PEM = "鍵ファイル作成";
        public const string PROCESS_NAME_CREATE_CERTREQ_CSR = "証明書要求ファイル作成";
        public const string PROCESS_NAME_CREATE_SELFCRT_CRT = "自己署名証明書ファイル作成";
        public const string PROCESS_NAME_PAIRING = "ペアリング";
        public const string PROCESS_NAME_TEST_CTAPHID_INIT = "CTAPHID_INITのテスト";

        // ファイル選択／保存ダイアログ用フィルター
        public const string FILTER_SELECT_PEM_PATH = "秘密鍵ファイル (*.pem)|*.pem";
        public const string FILTER_SELECT_CSR_PATH = "証明書要求ファイル (*.csr)|*.csr";
        public const string FILTER_SELECT_CRT_PATH = "証明書ファイル (*.crt)|*.crt";

        // デフォルトファイル名
        public const string DEFAULT_PEM_NAME = "U2FPrivKey";
        public const string DEFAULT_CSR_NAME = "U2FCertReq";
        public const string DEFAULT_CRT_NAME = "U2FSelfCer";
    }
}
