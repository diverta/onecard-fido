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
        public const string MSG_NONE = "";

        // メッセージリソース
        public const string MSG_USB_DETECT_FAILED = "USBデバイス検知の開始に失敗しました。";
        public const string MSG_USB_DETECT_STARTED = "USBデバイス検知を開始しました。";
        public const string MSG_USB_DETECT_END = "USBデバイス検知を終了しました。";
        public const string MSG_HID_REMOVED = "USB HIDデバイスが取り外されました。";
        public const string MSG_HID_CONNECTED = "USB HIDデバイスに接続されました。";

        // コマンド種別に対応する処理名称
        public const string PROCESS_NAME_PAIRING = "ペアリング";
        public const string PROCESS_NAME_ERASE_BONDS = "ペアリング情報削除";
        public const string PROCESS_NAME_CLIENT_PIN_SET = "PINコード新規設定";
        public const string PROCESS_NAME_CLIENT_PIN_CHANGE = "PINコード変更";
        public const string PROCESS_NAME_AUTH_RESET = "FIDO認証情報の消去";
        public const string PROCESS_NAME_BLE_DFU = "FIDO認証器のファームウェア更新";
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

        // BLE設定機能
        public const string MSG_PROMPT_INPUT_PAIRING_PASSCODE = "パスコードを６桁で入力してください";
        public const string MSG_PROMPT_INPUT_PAIRING_PASSCODE_NUM = "パスコードを数字で入力してください";
        public const string MSG_ERASE_BONDS = "FIDO認証器からペアリング情報をすべて削除します。";
        public const string MSG_PROMPT_ERASE_BONDS = "削除後はBLE経由のユーザー登録／ログインができなくなります。\n削除処理を実行しますか？";
        public const string MSG_ERASE_BONDS_COMMAND_ERROR = "ペアリング情報削除コマンドの実行に失敗しました。";

        // FIDO設定機能
        public const string MSG_PROMPT_INPUT_NEW_PIN = "新しいPINコードを４〜16桁で入力してください";
        public const string MSG_PROMPT_INPUT_NEW_PIN_CONFIRM = "新しいPINコード（確認用）を４〜16桁で入力してください";
        public const string MSG_PROMPT_INPUT_OLD_PIN = "変更前のPINコードを４〜16桁で入力してください";
        public const string MSG_PROMPT_INPUT_NEW_PIN_NUM = "新しいPINコードを数字で入力してください";
        public const string MSG_PROMPT_INPUT_NEW_PIN_CONF_NUM = "新しいPINコード（確認用）を数字で入力してください";
        public const string MSG_PROMPT_INPUT_OLD_PIN_NUM = "変更前のPINコードを数字で入力してください";
        public const string MSG_PROMPT_INPUT_PIN_CONFIRM_CRCT = "確認用のPINコードを正しく入力してください";
        public const string MSG_CTAP2_ERR_SSKEY_GENERATE_FOR_SET_PIN_CODE = "PINコード設定のための共通鍵生成処理が失敗しました。";
        public const string MSG_CLEAR_PIN_CODE = "FIDO認証器に設定された認証情報を消去します。";
        public const string MSG_PROMPT_CLEAR_PIN_CODE = "消去後はFIDO認証器によるログインができなくなります。\n（インストールされた鍵・証明書はそのまま残ります）\n\nFIDO認証情報の消去処理を実行しますか？";
        public const string MSG_CLEAR_PIN_CODE_COMMENT1 = "  ユーザー確認が必要となりますので、";
        public const string MSG_CLEAR_PIN_CODE_COMMENT2 = "  FIDO認証器上のユーザー確認LEDが高速点滅したら、";
        public const string MSG_CLEAR_PIN_CODE_COMMENT3 = "  ボタンを１回押してください.";
        public const string MSG_AUTH_RESET_COMMAND_ERROR = "FIDO認証情報消去コマンドの実行に失敗しました。";

        // DFU関連メッセージ
        public const string MSG_DFU_PROMPT_START_USB_DFU = "OKボタンをクリックすると、\nファームウェア更新処理が開始されます。\n\n処理が完了するまでは、FIDO認証器を\nUSBポートから外さないでください。";
        public const string MSG_DFU_PROMPT_START_BLE_DFU = "OKボタンをクリックすると、\nファームウェア更新処理が開始されます。\n\n処理が完了するまでは、FIDO認証器の\n電源をOnにしたままにして下さい。";
        public const string MSG_DFU_PRE_PROCESS = "ファームウェア更新機能の内部処理中です";
        public const string MSG_DFU_SUB_PROCESS_FAILED = "ファームウェア更新機能の内部処理が失敗しました。";
        public const string MSG_DFU_VERSION_INFO_GET_FAILED = "FIDO認証器ファームウェアのバージョンが取得できませんでした。";
        public const string MSG_DFU_IMAGE_NOT_AVAILABLE = "ファームウェア更新機能が利用できません。";
        public const string MSG_DFU_IMAGE_TRANSFER_FAILED = "更新ファームウェアの転送に失敗しました。";
        public const string MSG_DFU_IMAGE_TRANSFER_FAILED_WITH_RC = "更新ファームウェアの転送中に不明なエラー（rc={0}）が発生しました。";
        public const string MSG_DFU_IMAGE_TRANSFER_CANCELED = "更新ファームウェアの転送が中断されました。";
        public const string MSG_DFU_IMAGE_TRANSFER_SUCCESS = "更新ファームウェアの転送が完了しました。";
        public const string MSG_DFU_IMAGE_INSTALL_FAILED_WITH_RC = "更新ファームウェアの転送後に不明なエラー（rc={0}）が発生しました。";
        public const string MSG_DFU_IMAGE_ALREADY_INSTALLED = "更新ファームウェアが既に導入済みなので、ファームウェア更新処理を続行できません。";
        public const string MSG_DFU_TARGET_NOT_BOOTLOADER_MODE = "FIDO認証器をブートローダーモードに遷移させることができません。";
        public const string MSG_DFU_TARGET_BOOTLOADER_MODE = "FIDO認証器がブートローダーモードに遷移しました。";
        public const string MSG_DFU_TARGET_NOT_NORMAL_MODE = "FIDO認証器のファームウェア再始動に失敗しました。";
        public const string MSG_DFU_TARGET_NORMAL_MODE = "FIDO認証器のファームウェアが再始動しました。";
        public const string MSG_DFU_UPDATE_IMAGE_FILE_NOT_EXIST = "ファームウェア更新イメージファイルが存在しません。";
        public const string MSG_DFU_UPDATE_VERSION_UNKNOWN = "FIDO認証器ファームウェアの更新バージョンが不明です。";
        public const string MSG_DFU_CURRENT_VERSION_UNKNOWN = "FIDO認証器ファームウェアの現在バージョンが不明です。";
        public const string MSG_DFU_CURRENT_VERSION_ALREADY_NEW = "FIDO認証器のファームウェア (現在のバージョン: {0}) を、バージョン{1}に更新することはできません。";
        public const string MSG_DFU_CURRENT_VERSION_OLD_FIRMWARE = "FIDO認証器ファームウェアをバージョン{0}に更新するためには、ファームウェアをバージョン{1}以降に更新してください。";
        public const string MSG_DFU_FIRMWARE_VERSION_UPDATED = "FIDO認証器ファームウェアのバージョンが{0}に更新されました。";
        public const string MSG_DFU_FIRMWARE_VERSION_UPDATED_FAILED = "FIDO認証器ファームウェアのバージョンを{0}に更新できませんでした。";
        public const string MSG_DFU_PROCESS_CONNECT_FAILED = "更新ファームウェア転送サービスの接続に失敗しました。";
        public const string MSG_DFU_PROCESS_REQUEST_FAILED = "更新ファームウェア転送サービスへのデータ送信に失敗しました。";
        public const string MSG_DFU_PROCESS_RESPONSE_FAILED = "更新ファームウェア転送サービスからのデータ受信に失敗しました。";
        public const string MSG_DFU_PROCESS_VERIFY_CHECKSUM_FAILED = "更新ファームウェア転送時、送信データのチェックサム検証に失敗しました。";
        public const string MSG_DFU_PROCESS_VERIFY_SENDSIZE_FAILED = "更新ファームウェア転送時、送信データ長の検証に失敗しました。";
        public const string MSG_DFU_PROCESS_TIMEOUT = "FIDO認証器ファームウェアの更新処理がタイムアウトしました。";
        public const string MSG_DFU_PROCESS_TITLE_GOING = "ファームウェアを更新しています";
        public const string MSG_DFU_PROCESS_TRANSFER_IMAGE = "更新ファームウェアを転送中です。";
        public const string MSG_DFU_PROCESS_TRANSFER_IMAGE_FORMAT = "更新ファームウェアを転送中（{0}％）";
        public const string MSG_DFU_PROCESS_WAITING_UPDATE = "転送された更新ファームウェアの反映を待機中です。";
        public const string MSG_DFU_PROCESS_CONFIRM_VERSION = "転送された更新ファームウェアのバージョンを確認中です。";
        public const string MSG_DFU_SLOT_INFO_GET_FAILED = "FIDO認証器のプログラム領域情報が取得できませんでした。";
        public const string MSG_DFU_CHANGE_IMAGE_UPDATE_MODE_FAILED = "FIDO認証器ファームウェアの反映要求に失敗しました。";
        public const string MSG_DFU_RESET_APPLICATION_FAILED = "FIDO認証器ファームウェアの再始動要求に失敗しました。";
        public const string MSG_PROMPT_START_BLE_DFU_PROCESS = "ファームウェア更新処理を開始しますか？";
        public const string MSG_COMMENT_START_BLE_DFU_PROCESS = "BLEペアリングの済んだFIDO認証器が\nBLEペリフェラルモードになっているのを\n確認した後、Yesボタンをクリックすると、\nBLE経由でファームウェア更新処理が\n開始されます。\n\nFIDO認証器は、バージョン0.4.0以降の\nファームウェアが導入済みのものをご利用\nください。";

        // OpenPGP機能設定関連
        public const string MSG_FORMAT_OPENPGP_WILL_PROCESS = "{0}を実行します。";
        public const string MSG_FORMAT_OPENPGP_ITEM_FOR_CONF = "{0}（確認）";
        public const string MSG_FORMAT_OPENPGP_ITEM_FOR_CONFIRM = "{0}（確認用）";
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
        public const string MSG_LABEL_ITEM_PGP_RESET_CODE = "リセットコード";
        public const string MSG_LABEL_ITEM_CUR_PIN = "現在のPIN番号";
        public const string MSG_LABEL_ITEM_NEW_PIN = "新しいPIN番号";
        public const string MSG_LABEL_ITEM_CUR_ADMPIN = "現在の管理用PIN番号";
        public const string MSG_LABEL_ITEM_NEW_ADMPIN = "新しい管理用PIN番号";
        public const string MSG_LABEL_ITEM_CUR_RESET_CODE = "現在のリセットコード";
        public const string MSG_LABEL_ITEM_NEW_RESET_CODE = "新しいリセットコード";
        public const string MSG_LABEL_COMMAND_OPENPGP_INSTALL_KEYS = "PGP秘密鍵のインストール";
        public const string MSG_LABEL_COMMAND_OPENPGP_RESET = "設定情報の消去";
        public const string MSG_LABEL_COMMAND_OPENPGP_CHANGE_PIN = "PIN番号変更";
        public const string MSG_LABEL_COMMAND_OPENPGP_CHANGE_ADMIN_PIN = "管理用PIN番号変更";
        public const string MSG_LABEL_COMMAND_OPENPGP_UNBLOCK_PIN = "PIN番号リセット";
        public const string MSG_LABEL_COMMAND_OPENPGP_SET_RESET_CODE = "リセットコード変更";
        public const string MSG_LABEL_COMMAND_OPENPGP_UNBLOCK = "リセットコードによるPIN番号リセット";

        // ヘルスチェック機能
        public const string MSG_PROMPT_INPUT_HCHECK_PIN = "PINコードを４〜16桁で入力してください";
        public const string MSG_PROMPT_INPUT_HCHECK_PIN_NUM = "PINコードを数字で入力してください";
        public const string MSG_CMDTST_INVALID_NONCE = "CTAPHID_INITコマンドが失敗しました。";
        public const string MSG_CMDTST_INVALID_PING = "CTAPHID_PINGコマンドが失敗しました。";
        public const string MSG_CMDTST_INVALID_CTAPHID_CMD = "CTAPHIDコマンドのレスポンスが不正です。";

        // ヘルスチェック関連メッセージ
        public const string MSG_CTAP2_ERR_PIN_AUTH_SSKEY_GENERATE = "PIN認証のための共通鍵生成処理が失敗しました。";
        public const string MSG_CTAP2_ERR_PIN_AUTH_TOKEN_GET = "PIN認証で発行されたトークンの取得に失敗しました。";
        public const string MSG_CTAP2_ERR_PIN_INVALID = "入力されたPINコードが違います。正しいPINコードを入力してください。";
        public const string MSG_CTAP2_ERR_PIN_BLOCKED = "使用中のPINコードが無効となりました。新しいPINコードを設定し直してください。";
        public const string MSG_CTAP2_ERR_PIN_AUTH_BLOCKED = "PIN認証が無効となりました。認証器をUSBポートから取り外してください。";
        public const string MSG_CTAP2_ERR_PIN_NOT_SET = "PINコードが認証器に設定されていません。PINコードを新規設定してください。";
        public const string MSG_CTAP2_ERR_HMAC_INVALID = "ログインテスト時の暗号検証に失敗しました。";
        public const string MSG_HCHK_CTAP2_LOGIN_TEST_START = "ログインテストを開始します.";
        public const string MSG_HCHK_CTAP2_LOGIN_TEST_COMMENT1 = "  ユーザー所在確認が必要となりますので、";
        public const string MSG_HCHK_CTAP2_LOGIN_TEST_COMMENT2 = "  FIDO認証器上のユーザー所在確認LEDが点滅したら、";
        public const string MSG_HCHK_CTAP2_LOGIN_TEST_COMMENT3 = "  MAIN SWを１回押してください.";
        public const string MSG_HCHK_U2F_REGISTER_SUCCESS = "U2F Registerが成功しました。";
        public const string MSG_HCHK_U2F_AUTHENTICATE_START = "U2F Authenticateを開始します.";
        public const string MSG_HCHK_U2F_AUTHENTICATE_COMMENT1 = "  ユーザー所在確認が必要となりますので、";
        public const string MSG_HCHK_U2F_AUTHENTICATE_COMMENT2 = "  FIDO認証器上のユーザー所在確認LEDが点滅したら、";
        public const string MSG_HCHK_U2F_AUTHENTICATE_COMMENT3 = "  MAIN SWを１回押してください.";
        public const string MSG_HCHK_U2F_AUTHENTICATE_SUCCESS = "U2F Authenticateが成功しました。";
        public const string MSG_OCCUR_KEYHANDLE_ERROR = "キーハンドルが存在しません。再度ユーザー登録を実行してください。";
        public const string MSG_OCCUR_SKEYNOEXIST_ERROR = "鍵・証明書がインストールされていません。鍵・証明書インストール処理を実行してください。";
        public const string MSG_OCCUR_PAIRINGMODE_ERROR = "ペアリングモードでは、ペアリング実行以外の機能は使用できません。ペアリングモードを解除してから、機能を再度実行してください。";

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
        public const string MSG_BLE_U2F_SERVICE_FINDING = "FIDO BLEサービス({0})を検索します。";
        public const string MSG_BLE_U2F_SERVICE_NOT_FOUND = "FIDO BLEサービスが見つかりません。";
        public const string MSG_BLE_U2F_SERVICE_FOUND = "FIDO BLEサービスが見つかりました。";
        public const string MSG_U2F_DEVICE_CONNECT_FAILED = "FIDO認証器の接続に失敗しました。";
        public const string MSG_U2F_DEVICE_CONNECTED = "FIDO認証器に接続しました。";
        public const string MSG_U2F_DEVICE_DISCONNECTED = "FIDO認証器の接続が切断されました。";
        public const string MSG_BLE_CHARACT_NOT_DISCOVERED = "FIDO BLEサービスと通信できません。";
        public const string MSG_BLE_NOTIFICATION_FAILED = "FIDO BLEサービスからデータを受信できません。";
        public const string MSG_BLE_NOTIFICATION_START = "受信データの監視を開始します。";
        public const string MSG_BLE_NOTIFICATION_RETRY = "受信データ監視開始を再試行しています（{0}回目）";
        public const string MSG_REQUEST_SEND_FAILED = "リクエスト送信が失敗しました。";
        public const string MSG_REQUEST_SEND_FAILED_WITH_EXCEPTION = "リクエスト送信が失敗しました：{0}";
        public const string MSG_REQUEST_SEND_TIMED_OUT = "リクエスト送信がタイムアウトしました。";
        public const string MSG_REQUEST_SENT = "リクエストを送信しました。";
        public const string MSG_RESPONSE_RECEIVED = "レスポンスを受信しました。";
        public const string MSG_BLE_INVALID_PING = "BLE経由のPINGコマンドが失敗しました。";
        public const string MSG_BLE_SMP_SERVICE_FINDING = "BLE SMPサービス({0})を検索します。";
        public const string MSG_BLE_SMP_SERVICE_NOT_FOUND = "BLE SMPサービスが見つかりません。";
        public const string MSG_BLE_SMP_SERVICE_FOUND = "BLE SMPサービスが見つかりました。";
        public const string MSG_BLE_SMP_SERVICE_CONNECTED = "BLE SMPサービスに接続されました。";
        public const string MSG_BLE_SMP_SERVICE_DISCONNECTED = "BLE SMPサービスから切断されました。";
        public const string MSG_BLE_SMP_NOTIFICATION_FAILED = "BLE SMPサービスからデータを受信できません。";

        // BLEペアリング関連のメッセージ文言
        public const string MSG_BLE_PARING_ERR_BT_STATUS_CANNOT_GET = "Bluetooth状態を確認できません。";
        public const string MSG_BLE_PARING_ERR_BT_OFF = "Bluetoothがオフになっています。Bluetoothをオンにしてください。";
        public const string MSG_BLE_PARING_ERR_TIMED_OUT = "FIDO認証器が停止している可能性があります。FIDO認証器の電源を入れ、PCのUSBポートから外してください。";
        public const string MSG_BLE_PARING_ERR_PAIR_MODE = "FIDO認証器がペアリングモードでない可能性があります。FIDO認証器のMAIN SWを３秒間以上長押して、ペアリングモードに遷移させてください。";
        public const string MSG_BLE_PARING_ERR_UNKNOWN = "FIDO認証器とのペアリング時に不明なエラーが発生しました。";
    }
}
