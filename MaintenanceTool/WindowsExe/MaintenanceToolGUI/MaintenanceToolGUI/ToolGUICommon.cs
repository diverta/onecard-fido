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
        public const string MSG_ERASE_SKEY_CERT = "FIDO認証器から鍵・証明書・ユーザー登録情報をすべて削除します。";
        public const string MSG_PROMPT_ERASE_SKEY_CERT = "削除後はFIDO認証器によるユーザー登録／ログインができなくなります。\n削除処理を実行しますか？";
        public const string MSG_INSTALL_SKEY_CERT = "FIDO認証器に鍵・証明書をインストールします。";
        public const string MSG_PROMPT_INSTL_SKEY_CERT = "インストールを実行しますか？";
        public const string MSG_FORMAT_START_MESSAGE = "{0}を開始します。";
        public const string MSG_FORMAT_END_MESSAGE = "{0}が{1}しました。";
        public const string MSG_FIRMWARE_RESET_UNSUPP = "FIDO認証器のファームウェアをリセットさせることができません。";

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

        // ツール設定画面
        public const string MSG_LABEL_AUTH_PARAM_GET = "自動認証設定の読込";
        public const string MSG_LABEL_AUTH_PARAM_SET = "自動認証設定の書込";
        public const string MSG_LABEL_AUTH_PARAM_RESET = "自動認証設定の解除";
        public const string MSG_PROMPT_INPUT_UUID_STRING_LEN = "スキャン対象サービスUUIDを36桁で入力してください";
        public const string MSG_PROMPT_INPUT_UUID_STRING_PATTERN = "UUIDを正しい形式で入力してください。\n（例：422E0000-E141-11E5-A837-0800200C9A66）";
        public const string MSG_PROMPT_INPUT_UUID_SCAN_SEC_LEN = "スキャン秒数を1桁で入力してください";
        public const string MSG_PROMPT_INPUT_UUID_SCAN_SEC_NUM = "スキャン秒数を数字で入力してください";
        public const string MSG_PROMPT_INPUT_UUID_SCAN_SEC_RANGE = "スキャン秒数を1〜9の値で入力してください";
        public const string MSG_PROMPT_CLEAR_UUID_SCAN_PARAM = "解除後はBLEデバイススキャンによる自動認証ができなくなります。\n\n設定解除処理を実行しますか？";
        public const string MSG_PROMPT_WRITE_UUID_SCAN_PARAM_0 = "自動認証機能が無効化されているので、書込後もBLEデバイススキャンによる自動認証はできません。\n\n設定書込処理を実行しますか？";
        public const string MSG_PROMPT_WRITE_UUID_SCAN_PARAM_1 = "書込後はBLEデバイススキャンによる自動認証ができるようになります。\n\n設定書込処理を実行しますか？";
        public const string MSG_CLEAR_UUID_SCAN_PARAM = "FIDO認証器上の自動認証設定を解除します。";
        public const string MSG_WRITE_UUID_SCAN_PARAM = "FIDO認証器上の自動認証設定を変更します。";

        // BLE DFU関連
        public const string MSG_DFU_PRE_PROCESS = "ファームウェア更新機能の内部処理中です";
        public const string MSG_DFU_SUB_PROCESS_FAILED = "ファームウェア更新機能の内部処理が失敗しました。";
        public const string MSG_DFU_VERSION_INFO_GET_FAILED = "FIDO認証器ファームウェアのバージョンが取得できませんでした。";
        public const string MSG_DFU_CURRENT_VERSION_OLD_FIRMWARE = "FIDO認証器ファームウェアをバージョン{0}に更新するためには、ファームウェアをバージョン0.4.0以降に更新してください。";
        public const string MSG_DFU_SLOT_INFO_GET_FAILED = "FIDO認証器のプログラム領域情報が取得できませんでした。";
        public const string MSG_DFU_CHANGE_IMAGE_UPDATE_MODE_FAILED = "FIDO認証器ファームウェアの反映要求に失敗しました。";
        public const string MSG_DFU_RESET_APPLICATION_FAILED = "FIDO認証器ファームウェアの再始動要求に失敗しました。";
        public const string MSG_PROMPT_START_BLE_DFU_PROCESS = "ファームウェア更新処理を開始しますか？";
        public const string MSG_COMMENT_START_BLE_DFU_PROCESS = "BLEペアリングの済んだFIDO認証器が\nBLEペリフェラルモードになっているのを\n確認した後、Yesボタンをクリックすると、\nBLE経由でファームウェア更新処理が\n開始されます。\n\nFIDO認証器は、バージョン0.4.0以降の\nファームウェアが導入済みのものをご利用\nください。";

        // USB DFU関連
        public const string MSG_DFU_IMAGE_NOT_AVAILABLE = "ファームウェア更新機能が利用できません。";
        public const string MSG_DFU_IMAGE_NEW_NOT_AVAILABLE = "ファームウェア新規導入機能が利用できません。";
        public const string MSG_DFU_IMAGE_FILENAME_CANNOT_GET = "更新ファームウェアファイル名の取得に失敗しました。";
        public const string MSG_DFU_IMAGE_READ_FAILED = "更新ファームウェアの読込に失敗しました。";
        public const string MSG_DFU_IMAGE_TRANSFER_FAILED = "更新ファームウェアの転送に失敗しました。";
        public const string MSG_DFU_IMAGE_TRANSFER_FAILED_WITH_RC = "更新ファームウェアの転送中に不明なエラー（rc={0}）が発生しました。";
        public const string MSG_DFU_IMAGE_INSTALL_FAILED_WITH_RC = "更新ファームウェアの転送後に不明なエラー（rc={0}）が発生しました。";
        public const string MSG_DFU_IMAGE_TRANSFER_CANCELED = "更新ファームウェアの転送が中断されました。";
        public const string MSG_DFU_IMAGE_TRANSFER_SUCCESS = "更新ファームウェアの転送が完了しました。";
        public const string MSG_DFU_IMAGE_ALREADY_INSTALLED = "更新ファームウェアが既に導入済みなので、ファームウェア更新処理を続行できません。";
        public const string MSG_DFU_TARGET_NOT_BOOTLOADER_MODE = "FIDO認証器をブートローダーモードに遷移させることができません。";
        public const string MSG_DFU_TARGET_NOT_SECURE_BOOTLOADER = "FIDO認証器に、署名機能付きUSBブートローダーと、バージョン0.2.8以降のファームウェアをセットで導入してください。";
        public const string MSG_DFU_TARGET_NOT_CONNECTED = "FIDO認証器がブートローダーモードに遷移していません。";
        public const string MSG_DFU_TARGET_INVALID_SOFTDEVICE_VER = "FIDO認証器のUSBブートローダーを、最新バージョンに更新してください。";
        public const string MSG_DFU_UPDATE_IMAGE_FILE_NOT_EXIST = "ファームウェア更新イメージファイルが存在しません。";
        public const string MSG_DFU_UPDATE_VERSION_UNKNOWN = "FIDO認証器ファームウェアの更新バージョンが不明です。";
        public const string MSG_DFU_CURRENT_VERSION_UNKNOWN = "FIDO認証器ファームウェアの現在バージョンが不明です。";
        public const string MSG_DFU_CURRENT_VERSION_ALREADY_NEW = "FIDO認証器のファームウェア (現在のバージョン: {0}) を、バージョン{1}に更新することはできません。";
        public const string MSG_DFU_CURRENT_VERSION_OLD_USBBLD = "FIDO認証器ファームウェアをバージョン{0}に更新するためには、USBブートローダーを最新バージョンに更新してください。";
        public const string MSG_DFU_CURRENT_VERSION_GET_FAILED = "FIDO認証器ファームウェアの現在バージョン取得に失敗しました。";
        public const string MSG_DFU_FIRMWARE_VERSION_UPDATED = "FIDO認証器ファームウェアのバージョンが{0}に更新されました。";
        public const string MSG_DFU_FIRMWARE_VERSION_UPDATED_FAILED = "FIDO認証器ファームウェアのバージョンを{0}に更新できませんでした。";
        public const string MSG_DFU_PROCESS_TIMEOUT = "FIDO認証器ファームウェアの更新処理がタイムアウトしました。";
        public const string MSG_DFU_PROCESS_TITLE_GOING = "ファームウェアを更新しています";
        public const string MSG_DFU_PROCESS_TITLE_END = "ファームウェアの更新が完了しました";
        public const string MSG_DFU_PROCESS_TRANSFER_IMAGE = "更新ファームウェアを転送中です。";
        public const string MSG_DFU_PROCESS_TRANSFER_IMAGE_FORMAT = "更新ファームウェアを転送中（{0}％）";
        public const string MSG_DFU_PROCESS_WAITING_UPDATE = "転送された更新ファームウェアの反映を待機中です。";
        public const string MSG_DFU_PROCESS_CONFIRM_VERSION = "転送された更新ファームウェアのバージョンを確認中です。";
        public const string MSG_PROMPT_START_DFU_PROCESS = "ファームウェア新規導入処理を開始しますか？";
        public const string MSG_COMMENT_START_DFU_PROCESS = "署名機能付きブートローダーだけが導入された\nFIDO認証器をUSBポートに装着すると、\n自動的にブートローダーモードに遷移し、\n基板上の橙色・緑色LEDが連続点灯します。\n\nこの状態を確認したのち「はい」ボタンをクリックすると、\nファームウェア新規導入処理が開始されます。\n\nFIDO認証器は、最新版（MDBT50Q Dongle rev2.1.2）\nをご利用ください。";

        // OpenPGP機能設定関連
        public const string MSG_ERROR_OPENPGP_APPLET_SELECT_FAILED = "OpenPGP機能を使用することができません。";
        public const string MSG_ERROR_OPENPGP_GPG_VERSION_UNAVAIL = "バージョン「4.0.0」以降のGpg4winをインストールしてから実行してください。";
        public const string MSG_ERROR_OPENPGP_CREATE_TEMPDIR_FAIL = "作業用フォルダーを生成出来ませんでした。";
        public const string MSG_ERROR_OPENPGP_GENERATE_MAINKEY_FAIL = "PGP秘密鍵（主鍵）を生成出来ませんでした。";
        public const string MSG_ERROR_OPENPGP_GENERATE_SUB_KEY_FAIL = "PGP秘密鍵（副鍵）を生成出来ませんでした。";
        public const string MSG_ERROR_OPENPGP_EXPORT_PUBKEY_FAIL = "PGP公開鍵を指定フォルダーに生成出来ませんでした。";
        public const string MSG_ERROR_OPENPGP_BACKUP_FAIL = "バックアップファイルを指定フォルダーに生成出来ませんでした。";
        public const string MSG_ERROR_OPENPGP_EXPORT_BACKUP_FAIL = "PGP公開鍵／バックアップファイルの生成が失敗しました。";
        public const string MSG_ERROR_OPENPGP_TRANSFER_KEYS_FAIL = "生成したPGP秘密鍵（副鍵）を認証器に移動出来ませんでした。";
        public const string MSG_ERROR_OPENPGP_TRANSFER_SCRIPT_FAIL = "生成したPGP秘密鍵（副鍵）を認証器に移動するための内部処理が失敗しました。";
        public const string MSG_ERROR_OPENPGP_KEYS_ALREADY_STORED = "PGP秘密鍵（副鍵）が既に認証器に格納されているため、生成したPGP秘密鍵（副鍵）を移動出来ませんでした。";
        public const string MSG_ERROR_OPENPGP_REMOVE_TEMPDIR_FAIL = "作業用フォルダーが消去出来ませんでした。";
        public const string MSG_ERROR_OPENPGP_STATUS_COMMAND_FAIL = "OpenPGPステータス照会コマンドの実行に失敗しました。";
        public const string MSG_ERROR_OPENPGP_SELECTING_CARD_FAIL = "OpenPGP機能を認識出来ませんでした。\n認証器を一旦USBから取り外し、再度PCに装着した後、処理を再試行してください。";
        public const string MSG_ERROR_OPENPGP_SUBKEY_NOT_REMOVED = "PGP秘密鍵（副鍵）を認証器から正しく削除できませんでした。";
        public const string MSG_ERROR_OPENPGP_SUBKEY_REMOVE_FAIL = "PGP秘密鍵（副鍵）を認証器から削除時、不明なエラーが発生しました。";
        public const string MSG_FORMAT_OPENPGP_CREATED_TEMPDIR = "作業用フォルダーを新規に生成しました（{0}）。";
        public const string MSG_FORMAT_OPENPGP_GENERATED_MAIN_KEY = "PGP秘密鍵（主鍵）を新規に生成しました（鍵ID: {0}）。";
        public const string MSG_FORMAT_OPENPGP_EXPORT_PUBKEY_DONE = "PGP公開鍵ファイルを、指定フォルダー（{0}）に生成しました。";
        public const string MSG_FORMAT_OPENPGP_EXPORT_BACKUP_DONE = "PGP秘密鍵（主鍵）バックアップファイルを、指定フォルダー（{0}）に生成しました。";
        public const string MSG_FORMAT_OPENPGP_WILL_PROCESS = "{0}を実行します。";
        public const string MSG_FORMAT_OPENPGP_ITEM_FOR_CONF = "{0}（確認）";
        public const string MSG_FORMAT_OPENPGP_ITEM_FOR_CONFIRM = "{0}（確認用）";
        public const string MSG_FORMAT_OPENPGP_CARD_EDIT_PASSWD_ERR = "{0}時、不明なエラーが発生しました。";
        public const string MSG_FORMAT_OPENPGP_CARD_EDIT_PASSWD_NG = "入力した{0}が間違っている可能性があります。";
        public const string MSG_FORMAT_OPENPGP_PIN_VERIFY_ERR = "入力した{0}が間違っています。\n（あと{1}回リトライ可能です）";
        public const string MSG_OPENPGP_ADDED_SUB_KEYS = "PGP秘密鍵（副鍵）を新規に生成しました。";
        public const string MSG_OPENPGP_TRANSFERRED_KEYS_TO_DEVICE = "生成したPGP秘密鍵（副鍵）を認証器に移動しました。";
        public const string MSG_OPENPGP_REMOVED_TEMPDIR = "作業用フォルダーを消去しました。";
        public const string MSG_OPENPGP_INSTALL_PGP_KEY = "PGP秘密鍵を認証器にインストールします。";
        public const string MSG_PROMPT_SELECT_PGP_PUBKEY_FOLDER = "PGP公開鍵ファイルの出力先フォルダーを選択してください";
        public const string MSG_PROMPT_SELECT_PGP_BACKUP_FOLDER = "バックアップファイルの出力先フォルダーを選択してください";
        public const string MSG_PROMPT_INPUT_PGP_MUST_ENTRY = "{0}は必ず入力してください";
        public const string MSG_PROMPT_INPUT_PGP_ENTRY_DIGIT = "{0}は{1}～{2}文字で入力してください";
        public const string MSG_PROMPT_INPUT_PGP_ASCII_ENTRY = "{0}は半角文字で入力してください";
        public const string MSG_PROMPT_INPUT_PGP_ADDRESS_ENTRY = "{0}を正しく入力してください";
        public const string MSG_PROMPT_INPUT_PGP_PIN_DIGIT = "{0}は{1}桁で入力してください";
        public const string MSG_PROMPT_INPUT_PGP_ADMIN_PIN_DIGIT = "{0}を8桁で入力してください";
        public const string MSG_PROMPT_INPUT_PGP_ADMIN_PIN_NUM = "{0}を数字で入力してください";
        public const string MSG_PROMPT_INPUT_PGP_ADMIN_PIN_CONFIRM = "{0}を正しく入力してください";
        public const string MSG_PROMPT_INPUT_PGP_ENTRY_NOSP_BOTH_ENDS = "{0}の先頭または末尾の半角スペースを除去してください";
        public const string MSG_PROMPT_INSTALL_PGP_KEY = "インストールを実行しますか？";
        public const string MSG_PROMPT_OPENPGP_RESET = "OpenPGP機能の設定（鍵・PIN番号等）が全て削除され、OpenPGP機能が使用できなくなります。\n\n処理を開始しますか？";
        public const string MSG_PROMPT_OPENPGP_PIN_COMMAND = "処理を実行しますか？";
        public const string MSG_LABEL_PGP_REAL_NAME = "名前";
        public const string MSG_LABEL_PGP_MAIL_ADDRESS = "メールアドレス";
        public const string MSG_LABEL_PGP_COMMENT = "コメント";
        public const string MSG_LABEL_PGP_ADMIN_PIN = "OpenPGP機能の管理用PIN";
        public const string MSG_LABEL_PGP_ADMIN_PIN_CONFIRM = "OpenPGP機能の管理用PIN（確認）";
        public const string MSG_LABEL_ITEM_PGP_PIN = "PIN番号";
        public const string MSG_LABEL_ITEM_PGP_ADMIN_PIN = "管理用PIN番号";
        public const string MSG_LABEL_ITEM_PGP_RESET_CODE = "リセットコード";
        public const string MSG_LABEL_ITEM_CUR_PIN = "現在のPIN番号";
        public const string MSG_LABEL_ITEM_NEW_PIN = "新しいPIN番号";
        public const string MSG_LABEL_ITEM_CUR_ADMPIN = "現在の管理用PIN番号";
        public const string MSG_LABEL_ITEM_NEW_ADMPIN = "新しい管理用PIN番号";
        public const string MSG_LABEL_ITEM_CUR_RESET_CODE = "現在のリセットコード";
        public const string MSG_LABEL_ITEM_NEW_RESET_CODE = "新しいリセットコード";
        public const string MSG_LABEL_COMMAND_OPENPGP_INSTALL_KEYS = "PGP秘密鍵のインストール";
        public const string MSG_LABEL_COMMAND_OPENPGP_STATUS = "設定情報の参照";
        public const string MSG_LABEL_COMMAND_OPENPGP_RESET = "設定情報の消去";
        public const string MSG_LABEL_COMMAND_OPENPGP_CHANGE_PIN = "PIN番号変更";
        public const string MSG_LABEL_COMMAND_OPENPGP_CHANGE_ADMIN_PIN = "管理用PIN番号変更";
        public const string MSG_LABEL_COMMAND_OPENPGP_UNBLOCK_PIN = "PIN番号リセット";
        public const string MSG_LABEL_COMMAND_OPENPGP_SET_RESET_CODE = "リセットコード変更";
        public const string MSG_LABEL_COMMAND_OPENPGP_UNBLOCK = "リセットコードによるPIN番号リセット";
        public const string MSG_LABEL_COMMAND_OPENPGP_ADMIN_PIN_VERIFY = "管理用PIN番号の検証";

        // コマンド種別に対応する処理名称
        public const string PROCESS_NAME_ERASE_SKEY_CERT = "鍵・証明書の削除";
        public const string PROCESS_NAME_INSTALL_SKEY_CERT = "鍵・証明書インストール";
        public const string PROCESS_NAME_BLE_CTAP2_HEALTHCHECK = "BLE CTAP2ヘルスチェック";
        public const string PROCESS_NAME_BLE_U2F_HEALTHCHECK = "BLE U2Fヘルスチェック";
        public const string PROCESS_NAME_TEST_BLE_PING = "BLE PINGテスト";
        public const string PROCESS_NAME_PAIRING = "ペアリング";
        public const string PROCESS_NAME_HID_CTAP2_HEALTHCHECK = "HID CTAP2ヘルスチェック";
        public const string PROCESS_NAME_HID_U2F_HEALTHCHECK = "HID U2Fヘルスチェック";
        public const string PROCESS_NAME_TEST_CTAPHID_PING = "HID PINGテスト";
        public const string PROCESS_NAME_GET_FLASH_STAT = "Flash ROM情報取得";
        public const string PROCESS_NAME_GET_VERSION_INFO = "バージョン情報取得";
        public const string PROCESS_NAME_CLIENT_PIN_SET = "PINコード新規設定";
        public const string PROCESS_NAME_CLIENT_PIN_CHANGE = "PINコード変更";
        public const string PROCESS_NAME_AUTH_RESET = "PINコード解除";
        public const string PROCESS_NAME_BLE_DFU = "FIDO認証器のファームウェア更新(BLE)";
        public const string PROCESS_NAME_USB_DFU = "FIDO認証器のファームウェア更新(USB)";
        public const string PROCESS_NAME_ERASE_BONDS = "ペアリング情報削除";
        public const string PROCESS_NAME_BOOT_LOADER_MODE = "ブートローダーモード遷移";
        public const string PROCESS_NAME_FIRMWARE_RESET = "認証器のリセット";
        public const string PROCESS_NAME_OPENPGP_INSTALL_KEYS = "PGP秘密鍵インストール";
        public const string PROCESS_NAME_OPENPGP_STATUS = "OpenPGP設定情報取得";
        public const string PROCESS_NAME_OPENPGP_RESET = "OpenPGP機能リセット";

        // ファイル選択／保存ダイアログ用フィルター
        public const string FILTER_SELECT_PEM_PATH = "秘密鍵ファイル (*.pem)|*.pem";
        public const string FILTER_SELECT_CRT_PATH = "証明書ファイル (*.crt)|*.crt";

        // 起動時のメッセージ文言
        public const string MSG_INVALID_USER_ROLL = "このツールは、管理者として実行してください。\n\nプログラムアイコンを右クリックして、\nメニューから「管理者として実行」を選択します。";
        public const string MSG_ERROR_DOUBLE_START = "既に起動されています。";

        // PINコードの最小／最大桁数
        public const int PIN_CODE_SIZE_MIN = 4;
        public const int PIN_CODE_SIZE_MAX = 16;

        // 自動認証設定で使用
        // サービスUUID、スキャン秒数の桁数（固定）
        public const int AUTH_PARAM_UUID_STRING_SIZE = 36;
        public const int AUTH_PARAM_UUID_SCAN_SEC_SIZE = 1;
    }
}
