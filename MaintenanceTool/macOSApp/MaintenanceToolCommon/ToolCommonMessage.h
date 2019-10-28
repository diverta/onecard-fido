//
//  ToolCommonMessage.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2018/02/26.
//
#ifndef ToolCommonMessage_h
#define ToolCommonMessage_h

#pragma mark - 共通
#define MSG_INVALID_FIELD           @"入力値が不正です。"
#define MSG_INVALID_FIELD_SIZE      @"入力値の長さが不正です。"
#define MSG_INVALID_FILE_PATH       @"ファイルが存在しません。"
#define MSG_INVALID_OUT_OF_RANGE    @"入力値の範囲が不正です。"
#define MSG_NOT_NUMERIC             @"入力値が数字ではありません。"
#define MSG_BUTTON_SELECT           @"選択"
#define MSG_BUTTON_CREATE           @"作成"
#define MSG_SUCCESS                 @"成功"
#define MSG_FAILURE                 @"失敗"

#pragma mark - ホーム画面
#define MSG_PROMPT_SELECT_PKEY_PATH @"秘密鍵ファイル(PEM)のパスを選択してください"
#define MSG_PROMPT_SELECT_CRT_PATH  @"証明書ファイル(CRT)のパスを選択してください"
#define MSG_ERASE_SKEY_CERT         @"FIDO認証器から鍵・証明書・キーハンドルをすべて削除します。"
#define MSG_PROMPT_ERASE_SKEY_CERT  @"削除後はFIDO認証器によるユーザー登録／ログインができなくなります。\n削除処理を実行しますか？"
#define MSG_OCCUR_BLECONN_ERROR     @"BLE接続エラーが発生しました。"
#define MSG_OCCUR_KEYHANDLE_ERROR   @"キーハンドルが存在しません。再度ユーザー登録を実行してください。"
#define MSG_OCCUR_FDS_GC_ERROR      @"FIDO認証器のFlash ROM領域が一杯になり処理が中断されました(領域は自動再編成されます)。\n処理を再試行してください。"
#define MSG_OCCUR_UNKNOWN_BLE_ERROR @"BLEエラーが発生しました。処理を再試行してください。"
#define MSG_OCCUR_SKEYNOEXIST_ERROR @"鍵・証明書がインストールされていません。鍵・証明書インストール処理を実行してください。"
#define MSG_OCCUR_PAIRINGMODE_ERROR @"ペアリングモードでは、ペアリング実行以外の機能は使用できません。\nペアリングモードを解除してから、機能を再度実行してください。"
#define MSG_OCCUR_UNKNOWN_ERROR     @"不明なエラーが発生しました。"
#define MSG_FORMAT_START_MESSAGE    @"%1$@を開始します。"
#define MSG_FORMAT_END_MESSAGE      @"%1$@が%2$@しました。"

#pragma mark - コマンド種別に対応する処理名称
#define PROCESS_NAME_ERASE_SKEY_CERT                @"鍵・証明書・キーハンドル削除処理"
#define PROCESS_NAME_INSTALL_SKEY_CERT              @"鍵・証明書インストール"
#define PROCESS_NAME_HID_U2F_HEALTHCHECK            @"HID U2Fヘルスチェック"
#define PROCESS_NAME_BLE_U2F_HEALTHCHECK            @"BLE U2Fヘルスチェック"
#define PROCESS_NAME_HID_CTAP2_HEALTHCHECK          @"HID CTAP2ヘルスチェック"
#define PROCESS_NAME_BLE_CTAP2_HEALTHCHECK          @"BLE CTAP2ヘルスチェック"
#define PROCESS_NAME_PAIRING                        @"ペアリング"
#define PROCESS_NAME_TEST_CTAPHID_PING              @"HID PINGテスト"
#define PROCESS_NAME_TEST_BLE_PING                  @"BLE PINGテスト"
#define PROCESS_NAME_GET_FLASH_STAT                 @"Flash ROM情報取得"
#define PROCESS_NAME_GET_VERSION_INFO               @"バージョン情報取得"
#define PROCESS_NAME_CLIENT_PIN_SET                 @"PINコード新規設定"
#define PROCESS_NAME_CLIENT_PIN_CHANGE              @"PINコード変更"
#define PROCESS_NAME_AUTH_RESET                     @"PINコード解除"

#pragma mark - ToolCommandクラス専用メッセージ
#define MSG_INVALID_SKEY_LENGTH_IN_PEM      @"鍵ファイルに格納された秘密鍵の長さが不正です。"
#define MSG_INVALID_SKEY_HEADER_IN_PEM      @"鍵ファイルに格納された秘密鍵のヘッダーが不正です。"
#define MSG_CANNOT_READ_SKEY_PEM_FILE       @"鍵ファイルを読み込むことができません。"
#define MSG_INVALID_SKEY_CONTENT_IN_PEM     @"鍵ファイルの内容が不正です。"
#define MSG_CANNOT_READ_CERT_CRT_FILE       @"証明書ファイルを読み込むことができません。"
#define MSG_INVALID_CERT_LENGTH_IN_CRT      @"証明書ファイルに格納されたデータの長さが不正です。"
#define MSG_READ_NBYTES_FROM_CRT_FILE       @"証明書ファイル(%ldバイト)を読込みました。"

#pragma mark - ToolBLECentralクラス専用メッセージ
#define MSG_U2F_DEVICE_SCAN_START           @"FIDO認証器のスキャンを開始します。"
#define MSG_U2F_DEVICE_SCAN_STOPPED         @"FIDO認証器のスキャンを停止しました。"
#define MSG_U2F_DEVICE_SCAN_TIMEOUT         @"FIDO認証器のスキャンがタイムアウトしました。"
#define MSG_U2F_DEVICE_CONNREQ_TIMEOUT      @"FIDO認証器の接続要求がタイムアウトしました。"
#define MSG_U2F_DEVICE_CONNECTED            @"FIDO認証器に接続しました。"
#define MSG_U2F_DEVICE_CONNECT_FAILED       @"FIDO認証器の接続に失敗しました。"
#define MSG_U2F_DEVICE_DISCONNECTED         @"FIDO認証器の接続が切断されました。"
#define MSG_BLE_SERVICE_NOT_DISCOVERED      @"BLEサービスが見つかりません。"
#define MSG_BLE_U2F_SERVICE_FOUND           @"FIDO BLEサービスが見つかりました。"
#define MSG_BLE_U2F_SERVICE_NOT_FOUND       @"FIDO BLEサービスが見つかりません。"
#define MSG_BLE_CHARACT_NOT_DISCOVERED      @"FIDO BLEサービスと通信できません。"
#define MSG_BLE_CHARACT_NOT_EXIST           @"FIDO BLEサービスからデータを受信できる項目が存在しません。"
#define MSG_BLE_NOTIFICATION_FAILED         @"FIDO BLEサービスからデータを受信できません。"
#define MSG_BLE_NOTIFICATION_START          @"受信データの監視を開始します。"
#define MSG_BLE_NOTIFICATION_STOP           @"受信データの監視を停止します。"
#define MSG_BLE_INVALID_PING                @"BLE経由のPINGコマンドが失敗しました。"
#define MSG_REQUEST_SENT                    @"リクエストを送信しました。"
#define MSG_REQUEST_TIMEOUT                 @"リクエストがタイムアウトしました。"
#define MSG_REQUEST_SEND_FAILED             @"リクエスト送信が失敗しました。"
#define MSG_RESPONSE_RECEIVE_FAILED         @"レスポンスを受信できませんでした。"
#define MSG_DISCOVER_U2F_SERVICES_TIMEOUT   @"FIDO BLEサービスの検索がタイムアウトしました。"
#define MSG_DISCOVER_U2F_CHARAS_TIMEOUT     @"FIDO BLEサービス送受信項目の検索がタイムアウトしました。"
#define MSG_SUBSCRIBE_U2F_STATUS_TIMEOUT    @"FIDO BLEサービス受信項目の監視ステータス更新がタイムアウトしました。"

#pragma mark - 接続再試行関連メッセージ
#define MSG_BLE_CONNECTION_RETRY_WITH_CNT   @"処理中にBLE接続が消失しました。接続を再試行しています（%lu回目）"
#define MSG_BLE_CONNECTION_RETRY_END        @"処理中にBLE接続が消失しましたが、接続再試行の上限回数に達したため終了します。"

#pragma mark - ヘルスチェック関連メッセージ
#define MSG_HCHK_U2F_REGISTER_SUCCESS       @"U2F Registerが成功しました。"
#define MSG_HCHK_U2F_AUTHENTICATE_START     @"U2F Authenticateを開始します."
#define MSG_HCHK_U2F_AUTHENTICATE_COMMENT1  @"  ユーザー所在確認が必要となりますので、"
#define MSG_HCHK_U2F_AUTHENTICATE_COMMENT2  @"  FIDO認証器上のユーザー所在確認LEDが点滅したら、"
#define MSG_HCHK_U2F_AUTHENTICATE_COMMENT3  @"  MAIN SWを１回押してください."
#define MSG_HCHK_U2F_AUTHENTICATE_SUCCESS   @"U2F Authenticateが成功しました。"

#pragma mark - ToolHIDCommandクラス専用メッセージ
#define MSG_HID_CMD_RESPONSE_TIMEOUT        @"認証器からの応答が受信できませんでした。"

#pragma mark - コマンドテスト関連メッセージ
#define MSG_CMDTST_INVALID_NONCE            @"CTAPHID_INITコマンドが失敗しました。"
#define MSG_CMDTST_INVALID_PING             @"CTAPHID_PINGコマンドが失敗しました。"
#define MSG_CMDTST_PROMPT_USB_PORT_SET      @"FIDO認証器をUSBポートに装着してから実行してください。"
#define MSG_CMDTST_MENU_NOT_SUPPORTED       @"このメニューは実行できません。"

#pragma mark - PIN設定画面
#define MSG_PROMPT_INPUT_NEW_PIN            @"新しいPINコードを４〜16桁で入力してください"
#define MSG_PROMPT_INPUT_NEW_PIN_CONFIRM    @"新しいPINコード（確認用）を４〜16桁で入力してください"
#define MSG_PROMPT_INPUT_OLD_PIN            @"変更前のPINコードを４〜16桁で入力してください"
#define MSG_PROMPT_INPUT_NEW_PIN_NUM        @"新しいPINコードを数字で入力してください"
#define MSG_PROMPT_INPUT_NEW_PIN_CONF_NUM   @"新しいPINコード（確認用）を数字で入力してください"
#define MSG_PROMPT_INPUT_OLD_PIN_NUM        @"変更前のPINコードを数字で入力してください"
#define MSG_PROMPT_INPUT_PIN_CONFIRM_CRCT   @"確認用のPINコードを正しく入力してください"
#define MSG_PROMPT_INPUT_CUR_PIN            @"PINコードを４〜16桁で入力してください"
#define MSG_PROMPT_INPUT_CUR_PIN_NUM        @"PINコードを数字で入力してください"
#define MSG_CLEAR_PIN_CODE                  @"FIDO認証器に設定されたPINコードを解除します。"
#define MSG_PROMPT_CLEAR_PIN_CODE           @"解除後はFIDO認証器によるログインができなくなります。\n（インストールされた鍵・証明書はそのまま残ります）\n\nPINコード解除処理を実行しますか？"
#define MSG_CLEAR_PIN_CODE_COMMENT1         @"  ユーザー確認が必要となりますので、"
#define MSG_CLEAR_PIN_CODE_COMMENT2         @"  FIDO認証器上のユーザー確認LEDが高速点滅したら、"
#define MSG_CLEAR_PIN_CODE_COMMENT3         @"  MAIN SWを１回押してください."

#pragma mark - PINコードチェック関連メッセージ
#define MSG_CTAP2_ERR_PIN_INVALID           @"入力されたPINコードが違います。正しいPINコードを入力してください。"
#define MSG_CTAP2_ERR_PIN_BLOCKED           @"使用中のPINコードが無効となりました。新しいPINコードを設定し直してください。"
#define MSG_CTAP2_ERR_PIN_AUTH_BLOCKED      @"PIN認証が無効となりました。認証器をUSBポートから取り外してください。"
#define MSG_CTAP2_ERR_PIN_NOT_SET           @"PINコードが認証器に設定されていません。PINコードを新規設定してください。"

#pragma mark - CTAP2ヘルスチェック関連メッセージ
#define MSG_HCHK_CTAP2_LOGIN_TEST_START     @"ログインテストを開始します."
#define MSG_HCHK_CTAP2_LOGIN_TEST_COMMENT1  @"  ユーザー所在確認が必要となりますので、"
#define MSG_HCHK_CTAP2_LOGIN_TEST_COMMENT2  @"  FIDO認証器上のユーザー所在確認LEDが点滅したら、"
#define MSG_HCHK_CTAP2_LOGIN_TEST_COMMENT3  @"  MAIN SWを１回押してください."

#pragma mark - Flash ROM情報取得関連メッセージ
#define MSG_FSTAT_REMAINING_RATE            @"Flash ROMの空き容量は%.1f％です。"
#define MSG_FSTAT_NON_REMAINING_RATE        @"Flash ROMの空き容量を取得できませんでした。"
#define MSG_FSTAT_CORRUPTING_AREA_NOT_EXIST @"破損している領域は存在しません。"
#define MSG_FSTAT_CORRUPTING_AREA_EXIST     @"破損している領域が存在します。"

#pragma mark - バージョン情報取得関連メッセージ
#define MSG_VERSION_INFO_HEADER             @"FIDO認証器のバージョン情報"
#define MSG_VERSION_INFO_DEVICE_NAME        @"  デバイス名: %@"
#define MSG_VERSION_INFO_FW_REV             @"  ファームウェアのバージョン: %@"
#define MSG_VERSION_INFO_HW_REV             @"  ハードウェアのバージョン: %@"

#pragma mark - BLEペアリング関連のメッセージ文言
#define MSG_BLE_PARING_ERR_BT_OFF           @"Bluetoothがオフになっています。Bluetoothをオンにしてください。"
#define MSG_BLE_PARING_ERR_TIMED_OUT        @"FIDO認証器が停止している可能性があります。FIDO認証器の電源を入れ、PCのUSBポートから外してください。"
#define MSG_BLE_PARING_ERR_PAIR_MODE        @"FIDO認証器がペアリングモードでない可能性があります。FIDO認証器のMAIN SWを３秒間以上長押して、ペアリングモードに遷移させてください。"
#define MSG_BLE_PARING_ERR_UNKNOWN          @"FIDO認証器とのペアリング時に不明なエラーが発生しました。"

#pragma mark - ツール設定画面
#define MSG_PROMPT_INPUT_UUID_STRING_LEN        @"スキャン対象サービスUUIDを36桁で入力してください"
#define MSG_PROMPT_INPUT_UUID_SCAN_SEC_LEN      @"スキャン秒数を1桁で入力してください"
#define MSG_PROMPT_INPUT_UUID_SCAN_SEC_NUM      @"スキャン秒数を数字で入力してください"
#define MSG_PROMPT_INPUT_UUID_SCAN_SEC_RANGE    @"スキャン秒数を1〜9の値で入力してください"

#endif /* ToolCommonMessage_h */
