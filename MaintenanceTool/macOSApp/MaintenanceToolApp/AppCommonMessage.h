//
//  AppCommonMessage.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2022/07/06.
//
#ifndef AppCommonMessage_h
#define AppCommonMessage_h

#pragma mark - 共通
#define MSG_INVALID_FIELD_SIZE      @"入力値の長さが不正です。"
#define MSG_INVALID_OUT_OF_RANGE    @"入力値の範囲が不正です。"
#define MSG_INVALID_PATTERN         @"入力値の形式が不正です。"
#define MSG_INVALID_FIELD           @"入力値が不正です。"
#define MSG_INVALID_FILE_PATH       @"ファイルが存在しません。"
#define MSG_NOT_NUMERIC             @"入力値が数字ではありません。"
#define MSG_BUTTON_SELECT           @"選択"
#define MSG_BUTTON_CREATE           @"作成"
#define MSG_SUCCESS                 @"成功"
#define MSG_FAILURE                 @"失敗"
#define MSG_NONE                    @""

#pragma mark - USB HID関連
#define MSG_USB_DETECT_FAILED                   @"USBデバイス検知の開始に失敗しました。"
#define MSG_USB_DETECT_STARTED                  @"USBデバイス検知を開始しました。"
#define MSG_HID_REMOVED                         @"USB HIDデバイスが取り外されました。"
#define MSG_HID_CONNECTED                       @"USB HIDデバイスに接続されました。"
#define MSG_HID_CMD_RESPONSE_TIMEOUT            @"認証器からの応答が受信できませんでした。"
#define MSG_HID_CMD_INIT_WRONG_NONCE            @"認証器から不正な応答が検出されました。"
#define MSG_PROMPT_USB_PORT_SET                 @"FIDO認証器をUSBポートに装着してから実行してください。"

#pragma mark - BLE関連
#define MSG_U2F_DEVICE_SCAN_START               @"FIDO認証器のスキャンを開始します。"
#define MSG_U2F_DEVICE_SCAN_STOPPED             @"FIDO認証器のスキャンを停止しました。"
#define MSG_U2F_DEVICE_ESTABLISH_CONN_TIMEOUT   @"FIDO認証器の接続処理がタイムアウトしました。"

#pragma mark - ホーム画面
#define MSG_APP_NAME                @"FIDO認証器管理ツール"
#define MSG_APP_LAUNCHED            @"FIDO認証器管理ツールを起動しました: Version %@"
#define MSG_APP_TERMINATED          @"FIDO認証器管理ツールを終了しました。"
#define MSG_APP_COPYRIGHT           @"Copyright (c) 2017-2023 Diverta Inc."
#define MSG_PROMPT_SELECT_PKEY_PATH @"秘密鍵ファイル(PEM)のパスを選択してください"
#define MSG_PROMPT_SELECT_CRT_PATH  @"証明書ファイル(CRT)のパスを選択してください"
#define MSG_ERASE_SKEY_CERT         @"FIDO認証器から鍵・証明書・ユーザー登録情報をすべて削除します。"
#define MSG_PROMPT_ERASE_SKEY_CERT  @"削除後はFIDO認証器によるユーザー登録／ログインができなくなります。\n削除処理を実行しますか？"
#define MSG_INSTALL_SKEY_CERT       @"FIDO認証器に鍵・証明書をインストールします。"
#define MSG_PROMPT_INSTL_SKEY_CERT  @"インストールを実行しますか？"
#define MSG_OCCUR_BLECONN_ERROR     @"BLE接続エラーが発生しました。"
#define MSG_OCCUR_KEYHANDLE_ERROR   @"ユーザー登録情報が存在しません。再度ユーザー登録を実行してください。"
#define MSG_OCCUR_FDS_GC_ERROR      @"FIDO認証器のFlash ROM領域が一杯になり処理が中断されました(領域は自動再編成されます)。\n処理を再試行してください。"
#define MSG_OCCUR_SKEYNOEXIST_ERROR @"鍵・証明書がインストールされていません。鍵・証明書インストール処理を実行してください。"
#define MSG_OCCUR_PAIRINGMODE_ERROR @"ペアリングモードでは、ペアリング実行以外の機能は使用できません。ペアリングモードを解除してから、機能を再度実行してください。"
#define MSG_OCCUR_UNKNOWN_ERROR_SW  @"不明なエラーが発生しました（SW=0x%04x）"
#define MSG_OCCUR_UNKNOWN_ERROR_ST  @"不明なエラーが発生しました（Status=0x%02x）"
#define MSG_OCCUR_UNKNOWN_ERROR_LEN @"応答データがありませんでした。"
#define MSG_FORMAT_APP_VERSION      @"Version %@"
#define MSG_FORMAT_START_MESSAGE    @"%1$@を開始します。"
#define MSG_FORMAT_END_MESSAGE      @"%1$@が%2$@しました。"
#define MSG_ERASE_BONDS             @"FIDO認証器からペアリング情報をすべて削除します。"
#define MSG_PROMPT_ERASE_BONDS      @"削除後はBLE経由のユーザー登録／ログインができなくなります。\n削除処理を実行しますか？"
#define MSG_BOOT_LOADER_MODE_UNSUPP @"FIDO認証器をブートローダーモードに遷移できません。"
#define MSG_BOOT_LOADER_MODE        @"FIDO認証器をブートローダーモードに遷移させます。"
#define MSG_FIRMWARE_RESET_UNSUPP   @"FIDO認証器のファームウェアを再起動させることができません。"

#pragma mark - コマンド種別に対応する処理名称
#define PROCESS_NAME_HID_U2F_HEALTHCHECK            @"HID U2Fヘルスチェック"
#define PROCESS_NAME_BLE_U2F_HEALTHCHECK            @"BLE U2Fヘルスチェック"
#define PROCESS_NAME_HID_CTAP2_HEALTHCHECK          @"HID CTAP2ヘルスチェック"
#define PROCESS_NAME_BLE_CTAP2_HEALTHCHECK          @"BLE CTAP2ヘルスチェック"
#define PROCESS_NAME_PAIRING                        @"ペアリング"
#define PROCESS_NAME_UNPAIRING_REQUEST              @"ペアリング解除要求"
#define PROCESS_NAME_TEST_CTAPHID_PING              @"HID PINGテスト"
#define PROCESS_NAME_TEST_BLE_PING                  @"BLE PINGテスト"
#define PROCESS_NAME_GET_FLASH_STAT                 @"Flash ROM情報取得"
#define PROCESS_NAME_GET_VERSION_INFO               @"バージョン情報取得"
#define PROCESS_NAME_CLIENT_PIN_SET                 @"PINコード新規設定"
#define PROCESS_NAME_CLIENT_PIN_CHANGE              @"PINコード変更"
#define PROCESS_NAME_AUTH_RESET                     @"FIDO認証情報の消去"
#define PROCESS_NAME_USB_DFU                        @"FIDO認証器のファームウェア更新"
#define PROCESS_NAME_BLE_DFU                        @"FIDO認証器のファームウェア更新"
#define PROCESS_NAME_ERASE_BONDS                    @"ペアリング情報削除"
#define PROCESS_NAME_BOOT_LOADER_MODE               @"ブートローダーモード遷移"
#define PROCESS_NAME_FIRMWARE_RESET                 @"認証器のファームウェア再起動"
#define PROCESS_NAME_CCID_PIV_CHANGE_PIN            @"PIV PINコード変更"
#define PROCESS_NAME_CCID_PIV_CHANGE_PUK            @"PIV PUKコード変更"
#define PROCESS_NAME_CCID_PIV_UNBLOCK_PIN           @"PIV PIN解除"
#define PROCESS_NAME_CCID_PIV_RESET                 @"PIV機能リセット"
#define PROCESS_NAME_CCID_PIV_IMPORT_KEY            @"鍵・証明書インストール（PIV機能）"
#define PROCESS_NAME_CCID_PIV_SET_CHUID             @"PIV CHUID設定"
#define PROCESS_NAME_CCID_PIV_STATUS                @"PIV設定情報取得"
#define PROCESS_NAME_OPENPGP_INSTALL_KEYS           @"PGP秘密鍵インストール"
#define PROCESS_NAME_OPENPGP_STATUS                 @"OpenPGP設定情報取得"
#define PROCESS_NAME_OPENPGP_RESET                  @"OpenPGP機能リセット"
#define PROCESS_NAME_RTCC_GET_TIMESTAMP             @"認証器の現在時刻参照"
#define PROCESS_NAME_RTCC_SET_TIMESTAMP             @"認証器の現在時刻設定"
#define PROCESS_NAME_PIV_SETTINGS                   @"PIV設定情報"
#define PROCESS_NAME_OPENPGP_SETTINGS               @"OpenPGP設定情報"
#define PROCESS_NAME_INSTALL_ATTESTATION            @"鍵・証明書インストール"
#define PROCESS_NAME_REMOVE_ATTESTATION             @"鍵・証明書の削除"

#pragma mark - ToolBLECentralクラス専用メッセージ
#define MSG_U2F_DEVICE_SCAN_TIMEOUT         @"FIDO認証器のスキャンがタイムアウトしました。"
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
#define MSG_RESPONSE_RECEIVED               @"レスポンスを受信しました。"
#define MSG_RESPONSE_RECEIVE_FAILED         @"レスポンスを受信できませんでした。"

#pragma mark - ヘルスチェック関連メッセージ
#define MSG_HCHK_U2F_REGISTER_SUCCESS       @"U2F Registerが成功しました。"
#define MSG_HCHK_U2F_AUTHENTICATE_START     @"U2F Authenticateを開始します."
#define MSG_HCHK_U2F_AUTHENTICATE_COMMENT1  @"  ユーザー所在確認が必要となりますので、"
#define MSG_HCHK_U2F_AUTHENTICATE_COMMENT2  @"  FIDO認証器上の緑色LEDが点滅したら、"
#define MSG_HCHK_U2F_AUTHENTICATE_COMMENT3  @"  ボタンを１回押してください."
#define MSG_HCHK_U2F_AUTHENTICATE_SUCCESS   @"U2F Authenticateが成功しました。"

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
#define MSG_CLEAR_PIN_CODE                  @"FIDO認証器に設定された認証情報を消去します。"
#define MSG_PROMPT_CLEAR_PIN_CODE           @"消去後はFIDO認証器によるログインができなくなります。\n（インストールされた鍵・証明書はそのまま残ります）\n\nFIDO認証情報の消去処理を実行しますか？"
#define MSG_CLEAR_PIN_CODE_COMMENT1         @"  ユーザー確認が必要となりますので、"
#define MSG_CLEAR_PIN_CODE_COMMENT2         @"  FIDO認証器上の赤色LEDが高速点滅したら"
#define MSG_CLEAR_PIN_CODE_COMMENT3         @"  ボタンを１回押してください."

#pragma mark - PINコードチェック関連メッセージ
#define MSG_CTAP2_ERR_PIN_INVALID           @"入力されたPINコードが違います。正しいPINコードを入力してください。"
#define MSG_CTAP2_ERR_PIN_BLOCKED           @"使用中のPINコードが無効となりました。新しいPINコードを設定し直してください。"
#define MSG_CTAP2_ERR_PIN_AUTH_BLOCKED      @"PIN認証が無効となりました。認証器をUSBポートから取り外してください。"
#define MSG_CTAP2_ERR_PIN_NOT_SET           @"PINコードが認証器に設定されていません。PINコードを新規設定してください。"

#pragma mark - CTAP2ヘルスチェック関連メッセージ
#define MSG_HCHK_CTAP2_LOGIN_TEST_START     @"ログインテストを開始します."
#define MSG_HCHK_CTAP2_LOGIN_TEST_COMMENT1  @"  ユーザー所在確認が必要となりますので、"
#define MSG_HCHK_CTAP2_LOGIN_TEST_COMMENT2  @"  FIDO認証器上の緑色LEDが点滅したら、"
#define MSG_HCHK_CTAP2_LOGIN_TEST_COMMENT3  @"  ボタンを１回押してください."

#pragma mark - 時刻同期関連メッセージ
#define MSG_PROMPT_RTCC_SET_TIMESTAMP       @"PCの現在時刻を認証器に設定します。"
#define MSG_COMMENT_RTCC_SET_TIMESTAMP      @"処理を実行しますか？"

#pragma mark - Flash ROM情報取得関連メッセージ
#define MSG_FSTAT_REMAINING_RATE            @"Flash ROMの空き容量は%.1f％です。"
#define MSG_FSTAT_NON_REMAINING_RATE        @"Flash ROMの空き容量を取得できませんでした。"
#define MSG_FSTAT_CORRUPTING_AREA_NOT_EXIST @"破損している領域は存在しません。"
#define MSG_FSTAT_CORRUPTING_AREA_EXIST     @"破損している領域が存在します。"

#pragma mark - バージョン情報取得関連メッセージ
#define MSG_VERSION_INFO_HEADER             @"FIDO認証器のバージョン情報"
#define MSG_VERSION_INFO_DEVICE_NAME        @"  デバイス名: %@"
#define MSG_VERSION_INFO_FW_REV             @"  ファームウェアのバージョン: %@（%@）"
#define MSG_VERSION_INFO_HW_REV             @"  ハードウェアのバージョン: %@"

#pragma mark - BLEペアリング関連のメッセージ文言
#define MSG_BLE_PARING_ERR_BT_OFF           @"Bluetoothがオフになっています。Bluetoothをオンにしてください。"
#define MSG_BLE_PARING_ERR_TIMED_OUT        @"FIDO認証器が停止している可能性があります。FIDO認証器の電源を入れ、PCのUSBポートから外してください。"
#define MSG_BLE_PARING_ERR_PAIR_MODE        @"FIDO認証器がペアリングモードでない可能性があります。FIDO認証器のボタンを３秒間以上長押して、ペアリングモードに遷移させてください。"
#define MSG_BLE_PARING_ERR_UNKNOWN          @"FIDO認証器とのペアリング時に不明なエラーが発生しました。"
#define MSG_BLE_UNPAIRING_PREPARATION       @"ペアリング解除要求の準備中です。"
#define MSG_BLE_UNPAIRING_WAIT_DISCONNECT   @"Bluetooth環境設定から\nデバイス「%@」が\n削除されるのを待機しています。"
#define MSG_BLE_UNPAIRING_WAIT_SEC_FORMAT   @"あと %d 秒"
#define MSG_BLE_UNPAIRING_WAIT_CANCELED     @"ペアリング解除要求が中断されました。"
#define MSG_BLE_UNPAIRING_WAIT_DISC_TIMEOUT @"Bluetooth環境設定からの\nデバイス削除が検知されませんでした。\nペアリング解除要求を中止します。"

#pragma mark - BLE DFU関連
#define MSG_DFU_SUB_PROCESS_FAILED              @"ファームウェア更新機能の内部処理が失敗しました。"
#define MSG_DFU_VERSION_INFO_GET_FAILED         @"FIDO認証器ファームウェアのバージョンが取得できませんでした。"
#define MSG_DFU_CURRENT_VERSION_OLD_FIRMWARE    @"FIDO認証器ファームウェアをバージョン%@に更新するためには、ファームウェアをバージョン0.4.0以降に更新してください。"
#define MSG_DFU_SLOT_INFO_GET_FAILED            @"FIDO認証器のプログラム領域情報が取得できませんでした。"
#define MSG_DFU_CHANGE_IMAGE_UPDATE_MODE_FAILED @"FIDO認証器ファームウェアの反映要求に失敗しました。"
#define MSG_DFU_RESET_APPLICATION_FAILED        @"FIDO認証器ファームウェアの再始動要求に失敗しました。"
#define MSG_PROMPT_START_BLE_DFU_PROCESS        @"ファームウェア更新処理を開始しますか？"
#define MSG_COMMENT_START_BLE_DFU_PROCESS       @"BLEペアリングの済んだFIDO認証器が\nBLEペリフェラルモードになっているのを\n確認した後、Yesボタンをクリックすると、\nBLE経由でファームウェア更新処理が\n開始されます。\n\nFIDO認証器は、バージョン0.4.0以降の\nファームウェアが導入済みのものをご利用\nください。"

#pragma mark - USB DFU関連
#define MSG_DFU_IMAGE_NOT_AVAILABLE             @"ファームウェア更新機能が利用できません。"
#define MSG_DFU_IMAGE_FILENAME_CANNOT_GET       @"更新ファームウェアファイル名の取得に失敗しました。"
#define MSG_DFU_IMAGE_READ_FAILED               @"更新ファームウェアの読込に失敗しました。"
#define MSG_DFU_IMAGE_TRANSFER_FAILED           @"更新ファームウェアの転送に失敗しました。"
#define MSG_DFU_IMAGE_TRANSFER_CANCELED         @"更新ファームウェアの転送が中断されました。"
#define MSG_DFU_IMAGE_TRANSFER_SUCCESS          @"更新ファームウェアの転送が完了しました。"
#define MSG_DFU_IMAGE_ALREADY_INSTALLED         @"更新ファームウェアが既に導入済みなので、ファームウェア更新処理を続行できません。"
#define MSG_DFU_IMAGE_TRANSFER_FAILED_WITH_RC   @"更新ファームウェアの転送中に不明なエラー（rc=%d）が発生しました。"
#define MSG_DFU_IMAGE_INSTALL_FAILED_WITH_RC    @"更新ファームウェアの転送後に不明なエラー（rc=%d）が発生しました。"
#define MSG_DFU_TARGET_NOT_BOOTLOADER_MODE      @"FIDO認証器をブートローダーモードに遷移させることができません。"
#define MSG_DFU_TARGET_NOT_SECURE_BOOTLOADER    @"FIDO認証器に、署名機能付きUSBブートローダーと、バージョン0.2.8以降のファームウェアをセットで導入してください。"
#define MSG_DFU_TARGET_NOT_CONNECTED            @"FIDO認証器がブートローダーモードに遷移していません。"
#define MSG_DFU_TARGET_CONNECTION_FAILED        @"ファームウェア新規導入先のFIDO認証器に接続できませんでした。"
#define MSG_DFU_TARGET_INVALID_SOFTDEVICE_VER   @"FIDO認証器のUSBブートローダーを、最新バージョンに更新してください。"
#define MSG_DFU_UPDATE_IMAGE_FILE_NOT_EXIST     @"ファームウェア更新イメージファイルが存在しません。"
#define MSG_DFU_UPDATE_VERSION_UNKNOWN          @"FIDO認証器ファームウェアの更新バージョンが不明です。"
#define MSG_DFU_CURRENT_VERSION_UNKNOWN         @"FIDO認証器ファームウェアの現在バージョンが不明です。"
#define MSG_DFU_CURRENT_VERSION_ALREADY_NEW     @"FIDO認証器のファームウェア (現在のバージョン: %@) を、バージョン%@に更新することはできません。"
#define MSG_DFU_CURRENT_VERSION_OLD_USBBLD      @"FIDO認証器ファームウェアをバージョン%@に更新するためには、USBブートローダーを最新バージョンに更新してください。"
#define MSG_DFU_FIRMWARE_VERSION_UPDATED        @"FIDO認証器ファームウェアのバージョンが%@に更新されました。"
#define MSG_DFU_FIRMWARE_VERSION_UPDATED_FAILED @"FIDO認証器ファームウェアのバージョンを%@に更新できませんでした。"
#define MSG_DFU_PROCESS_TIMEOUT                 @"FIDO認証器ファームウェアの更新処理がタイムアウトしました。"
#define MSG_DFU_PROCESS_REBOOT_TIMEOUT          @"ファームウェア更新イメージの反映処理がタイムアウトしました。"
#define MSG_DFU_PROCESS_TITLE_GOING             @"ファームウェアを更新しています"
#define MSG_DFU_PROCESS_TITLE_END               @"ファームウェアの更新が完了しました"
#define MSG_DFU_PROCESS_TRANSFER_PREPARE        @"更新ファームウェア転送の準備中です。"
#define MSG_DFU_PROCESS_TRANSFER_IMAGE          @"更新ファームウェアを転送中です。"
#define MSG_DFU_PROCESS_TRANSFER_IMAGE_FORMAT   @"更新ファームウェアを転送中（%d％）"
#define MSG_DFU_PROCESS_WAITING_UPDATE          @"転送された更新ファームウェアの反映を待機中です。"
#define MSG_DFU_PROCESS_CONFIRM_VERSION         @"転送された更新ファームウェアのバージョンを確認中です。"
#define MSG_PROMPT_START_DFU_PROCESS            @"ファームウェア新規導入処理を開始しますか？"
#define MSG_COMMENT_START_DFU_PROCESS           @"署名機能付きブートローダーだけが導入された\nFIDO認証器をUSBポートに装着すると、\n自動的にブートローダーモードに遷移し、\n基板上の橙色・緑色LEDが連続点灯します。\n\nこの状態を確認したのち、Yesボタンをクリックすると、\nファームウェア新規導入処理が開始されます。\n\nFIDO認証器は、最新版（MDBT50Q Dongle rev2.1.2）\nをご利用ください。"
#define MSG_DESCRIPTION_START_DFU_PROCESS       @"OKボタンをクリックすると、\nファームウェア更新処理が開始されます。\n\n処理が完了するまでは、FIDO認証器を\nUSBポートから外さないでください。"

#pragma mark - USB CCID関連
#define MSG_CCID_SESSION_ALREADY_EXIST          @"CCIDインターフェースで別のセッションが存在します。"
#define MSG_CCID_INTERFACE_UNAVAILABLE          @"FIDO認証器にCCIDインターフェースが存在しません。"
#define MSG_CCID_DEVICE_UNAVAILABLE             @"FIDO認証器が利用できません（%@）"
#define MSG_CCID_DEVICE_CONNECT_ERROR           @"FIDO認証器との接続に失敗しました（%@）"
#define MSG_CCID_DEVICE_CONNECTED               @"FIDO認証器に接続しました（CCIDデバイス名: %@）"
#define MSG_CCID_DEVICE_DISCONNECTED            @"FIDO認証器から切断しました（CCIDデバイス名: %@）"
#define MSG_CCID_REQUEST_SEND_FAILED            @"FIDO認証器へのリクエスト送信が失敗しました（%@）"
#define MSG_ERROR_PIV_APPLET_SELECT_FAILED      @"PIV機能を使用することができません。"
#define MSG_ERROR_PIV_ADMIN_AUTH_REQ_FAILED     @"PIV管理機能認証（往路）が失敗しました。"
#define MSG_ERROR_PIV_ADMIN_AUTH_RES_FAILED     @"PIV管理機能認証（復路）が失敗しました。"
#define MSG_ERROR_PIV_ADMIN_AUTH_FUNC_FAILED    @"PIV管理機能認証の内部処理が失敗しました（%@）"
#define MSG_ERROR_PIV_ADMIN_AUTH_CHALLENGE_DIFF @"PIV管理機能認証が失敗しました（チャレンジが一致しません）。"
#define MSG_ERROR_PIV_WRONG_PIN                 @"%@が不正です。正しい%@を入力してください（残り%d回試行可能です）。"
#define MSG_ERROR_PIV_PIN_LOCKED                @"PINがすでに無効です。PIN解除を実行し、新しいPINを登録して下さい。"
#define MSG_ERROR_PIV_PUK_LOCKED                @"PUKがすでに無効です。PIV機能をリセットする必要があります。"
#define MSG_ERROR_PIV_RESET_FAIL                @"PINまたはPUKが未だ無効になっていません。"
#define MSG_ERROR_PIV_UNKNOWN                   @"不明なエラーが発生しました（SW=0x%04x）"
#define MSG_ERROR_PIV_SELECTING_CARD_FAIL       @"PIV機能を認識出来ませんでした。\n認証器を一旦USBから取り外し、再度PCに装着した後、処理を再試行してください。"
#define MSG_PROMPT_SELECT_PIV_PKEY_PEM_PATH     @"PIV認証用のPIV秘密鍵ファイル(PEM)を選択してください"
#define MSG_PROMPT_SELECT_PIV_CERT_PEM_PATH     @"PIV認証用のPIV証明書ファイル(PEM)を選択してください"
#define MSG_PROMPT_SELECT_PIV_PKEY_PEM_PATH_2   @"電子署名用のPIV秘密鍵ファイル(PEM)を選択してください"
#define MSG_PROMPT_SELECT_PIV_CERT_PEM_PATH_2   @"電子署名用のPIV証明書ファイル(PEM)を選択してください"
#define MSG_PROMPT_SELECT_PIV_PKEY_PEM_PATH_3   @"管理機能用のPIV秘密鍵ファイル(PEM)を選択してください"
#define MSG_PROMPT_SELECT_PIV_CERT_PEM_PATH_3   @"管理機能用のPIV証明書ファイル(PEM)を選択してください"
#define MSG_ERROR_PIV_PKEY_PEM_LOAD_FAILED      @"PIV秘密鍵ファイル読込処理が失敗しました（%@）"
#define MSG_ERROR_PIV_CERT_PEM_LOAD_FAILED      @"PIV証明書ファイル読込処理が失敗しました（%@）"
#define MSG_FORMAT_PIV_PKEY_PEM_LOADED          @"PIV秘密鍵ファイル（%@）を正常に読込みました。"
#define MSG_FORMAT_PIV_CERT_PEM_LOADED          @"PIV証明書ファイル（%@）を正常に読込みました。"
#define MSG_INSTALL_PIV_PKEY_CERT               @"PIV秘密鍵・証明書をインストールします。"
#define MSG_FORMAT_ERROR_PIV_IMPORT_PKEY_FAILED @"PIV秘密鍵（%@）のインポート処理が失敗しました（alg=%@）。"
#define MSG_FORMAT_ERROR_PIV_IMPORT_CERT_FAILED @"PIV証明書（%@）のインポート処理が失敗しました（alg=%@）。"
#define MSG_FORMAT_PIV_PKEY_PEM_IMPORTED        @"PIV秘密鍵（%@）を正常にインポートしました（alg=%@）。"
#define MSG_FORMAT_PIV_CERT_PEM_IMPORTED        @"PIV証明書（%@）を正常にインポートしました（alg=%@）。"
#define MSG_ERROR_PIV_IMPORT_CHUID_FAILED       @"PIV CHUIDインポート処理が失敗しました。"
#define MSG_ERROR_PIV_IMPORT_CCC_FAILED         @"PIV CCCインポート処理が失敗しました。"
#define MSG_PIV_CHUID_IMPORTED                  @"PIV CHUIDを正常にインポートしました。"
#define MSG_PIV_CCC_IMPORTED                    @"PIV CCCを正常にインポートしました。"
#define MSG_ERROR_PIV_PIN_RETRY_CNT_GET_FAILED  @"PIV PINリトライカウンターを取得できませんでした。"
#define MSG_ERROR_PIV_DATA_OBJECT_GET_FAILED    @"PIVデータオブジェクトを取得できませんでした（ID=0x%06x）。"
#define MSG_PIV_PIN_RETRY_CNT_GET               @"PIV PINリトライカウンターを取得しました（残り%d回試行可能です）。"
#define MSG_PIV_DATA_OBJECT_GET                 @"PIVデータオブジェクトを取得しました（ID=0x%06x）。"
#define MSG_ERROR_PIV_CERT_INFO_GET_FAILED      @"PIV証明書からの属性取得処理が失敗しました（%@）"
#define MSG_PROMPT_INPUT_PIV_PIN_PUK_DIGIT      @"%@を6〜8桁で入力してください"
#define MSG_PROMPT_INPUT_PIV_PIN_PUK_NUM        @"%@を数字で入力してください"
#define MSG_PROMPT_INPUT_PIV_PIN_PUK_CONFIRM    @"%@を正しく入力してください"
#define MSG_LABEL_CURRENT_PIN                   @"現在のPIN番号"
#define MSG_LABEL_CURRENT_PIN_FOR_CONFIRM       @"現在のPIN番号（確認用）"
#define MSG_LABEL_NEW_PIN                       @"新しいPIN番号"
#define MSG_LABEL_NEW_PIN_FOR_CONFIRM           @"新しいPIN番号（確認用）"
#define MSG_LABEL_CURRENT_PUK                   @"現在のPUK番号"
#define MSG_LABEL_NEW_PUK                       @"新しいPUK番号"
#define MSG_LABEL_NEW_PUK_FOR_CONFIRM           @"新しいPUK番号（確認用）"
#define MSG_FORMAT_WILL_PROCESS                 @"%@を実行します。"
#define MSG_FORMAT_PROCESS_INFORMATIVE          @"%@\n\n処理を開始しますか？"
#define MSG_PIV_INITIAL_SETTING                 @"ID設定の処理"
#define MSG_PROMPT_PIV_INITIAL_SETTING          @"新規にCHUID、CCCが設定されます。\n\n処理を開始しますか？"
#define MSG_PIV_CLEAR_SETTING                   @"設定情報の消去"
#define MSG_PROMPT_PIV_CLEAR_SETTING            @"PIV機能の設定（鍵・証明書・PIN番号等）が全て削除され、PIV機能が使用できなくなります。\n\n処理を開始しますか？"
#define MSG_PIV_INSTALL_PKEY_CERT               @"鍵・証明書ファイルのインストール"
#define MSG_FORMAT_PIV_LOAD_PKEY_FAILED         @"鍵ファイル（%@）の読込が失敗しました。"
#define MSG_FORMAT_PIV_LOAD_CERT_FAILED         @"証明書ファイル（%@）の読込が失敗しました。"
#define MSG_FORMAT_PIV_PKEY_CERT_ALGORITHM      @"鍵アルゴリズム（0x%02x）と証明書アルゴリズム（0x%02x）が異なっています。\n正しい組み合わせの鍵・証明書（%@）をご使用ください。"
#define MSG_PIV_CHANGE_PIN_NUMBER               @"PIN番号の変更"
#define MSG_PIV_CHANGE_PUK_NUMBER               @"PUK番号の変更"
#define MSG_PIV_RESET_PIN_NUMBER                @"PIN番号のリセット"
#define MSG_DESC_PIV_CHANGE_PIN_NUMBER          @"現在のPIN番号を、入力した新しいPIN番号に変更します。"
#define MSG_DESC_PIV_CHANGE_PUK_NUMBER          @"現在のPUK番号を、入力した新しいPUK番号に変更します。"
#define MSG_DESC_PIV_RESET_PIN_NUMBER           @"現在のPIN番号をリセットし、入力した新しいPIN番号に変更します。"
#define MSG_PIV_STATUS                          @"設定情報の取得"
#define MSG_PIV_KEY_SLOT_NAME_1                 @"PIV認証用"
#define MSG_PIV_KEY_SLOT_NAME_2                 @"電子署名用"
#define MSG_PIV_KEY_SLOT_NAME_3                 @"管理機能用"

#pragma mark - OpenPGP関連
#define MSG_ERROR_OPENPGP_APPLET_SELECT_FAILED  @"OpenPGP機能を使用することができません。"
#define MSG_ERROR_OPENPGP_GPG_VERSION_UNAVAIL   @"バージョン「2021.1」以降のGPG Suiteをインストールしてから実行してください。"
#define MSG_ERROR_OPENPGP_CREATE_TEMPDIR_FAIL   @"作業用フォルダーを生成出来ませんでした。"
#define MSG_ERROR_OPENPGP_READ_PARAM_TEMPL_FAIL @"パラメーターテンプレートをファイルから読込むことが出来ませんでした。"
#define MSG_ERROR_OPENPGP_WRITE_PARAM_FILE_FAIL @"GPGコマンドのパラメーターファイルを書き出すことが出来ませんでした。"
#define MSG_ERROR_OPENPGP_GENERATE_MAINKEY_FAIL @"PGP秘密鍵（主鍵）を生成出来ませんでした。"
#define MSG_ERROR_OPENPGP_GENERATE_SUB_KEY_FAIL @"PGP秘密鍵（副鍵）を生成出来ませんでした。"
#define MSG_ERROR_OPENPGP_EXPORT_PUBKEY_FAIL    @"PGP公開鍵を指定フォルダーに生成出来ませんでした。"
#define MSG_ERROR_OPENPGP_BACKUP_FAIL           @"バックアップファイルを指定フォルダーに生成出来ませんでした。"
#define MSG_ERROR_OPENPGP_EXPORT_BACKUP_FAIL    @"PGP公開鍵／バックアップファイルの生成が失敗しました。"
#define MSG_ERROR_OPENPGP_TRANSFER_KEYS_FAIL    @"生成したPGP秘密鍵（副鍵）を認証器に移動出来ませんでした。"
#define MSG_ERROR_OPENPGP_TRANSFER_SCRIPT_FAIL  @"生成したPGP秘密鍵（副鍵）を認証器に移動するための内部処理が失敗しました。"
#define MSG_ERROR_OPENPGP_KEYS_ALREADY_STORED   @"PGP秘密鍵（副鍵）が既に認証器に格納されているため、生成したPGP秘密鍵（副鍵）を移動出来ませんでした。"
#define MSG_ERROR_OPENPGP_REMOVE_TEMPDIR_FAIL   @"作業用フォルダーが消去出来ませんでした。"
#define MSG_ERROR_OPENPGP_STATUS_COMMAND_FAIL   @"OpenPGPステータス照会コマンドの実行に失敗しました。"
#define MSG_ERROR_OPENPGP_SELECTING_CARD_FAIL   @"OpenPGP機能を認識出来ませんでした。\n認証器を一旦USBから取り外し、再度PCに装着した後、処理を再試行してください。"
#define MSG_ERROR_OPENPGP_SUBKEY_NOT_REMOVED    @"PGP秘密鍵（副鍵）を認証器から正しく削除できませんでした。"
#define MSG_ERROR_OPENPGP_SUBKEY_REMOVE_FAIL    @"PGP秘密鍵（副鍵）を認証器から削除時、不明なエラーが発生しました。"
#define MSG_FORMAT_OPENPGP_CREATED_TEMPDIR      @"作業用フォルダーを新規に生成しました（%@）。"
#define MSG_FORMAT_OPENPGP_GENERATED_MAIN_KEY   @"PGP秘密鍵（主鍵）を新規に生成しました（鍵ID: %@）。"
#define MSG_FORMAT_OPENPGP_EXPORT_PUBKEY_DONE   @"PGP公開鍵ファイルを、指定フォルダー（%@）に生成しました。"
#define MSG_FORMAT_OPENPGP_EXPORT_BACKUP_DONE   @"PGP秘密鍵（主鍵）バックアップファイルを、指定フォルダー（%@）に生成しました。"
#define MSG_FORMAT_OPENPGP_WILL_PROCESS         @"%@を実行します。"
#define MSG_FORMAT_OPENPGP_ITEM_FOR_CONF        @"%@（確認）"
#define MSG_FORMAT_OPENPGP_ITEM_FOR_CONFIRM     @"%@（確認用）"
#define MSG_FORMAT_OPENPGP_CARD_EDIT_PASSWD_ERR @"%@時、不明なエラーが発生しました。"
#define MSG_FORMAT_OPENPGP_CARD_EDIT_PASSWD_NG  @"入力した%@が間違っている可能性があります。"
#define MSG_FORMAT_OPENPGP_PIN_VERIFY_ERR       @"入力した%@が間違っています。\n（あと%u回リトライ可能です）"
#define MSG_OPENPGP_ADDED_SUB_KEYS              @"PGP秘密鍵（副鍵）を新規に生成しました。"
#define MSG_OPENPGP_TRANSFERRED_KEYS_TO_DEVICE  @"生成したPGP秘密鍵（副鍵）を認証器に移動しました。"
#define MSG_OPENPGP_REMOVED_TEMPDIR             @"作業用フォルダーを消去しました。"
#define MSG_OPENPGP_INSTALL_PGP_KEY             @"PGP秘密鍵を認証器にインストールします。"
#define MSG_OPENPGP_ADMIN_PIN_VERIFIED          @"管理用PIN番号を検証しました。"
#define MSG_PROMPT_SELECT_PGP_PUBKEY_FOLDER     @"PGP公開鍵ファイルの出力先フォルダーを選択してください"
#define MSG_PROMPT_SELECT_PGP_BACKUP_FOLDER     @"バックアップファイルの出力先フォルダーを選択してください"
#define MSG_PROMPT_INPUT_PGP_MUST_ENTRY         @"%@は必ず入力してください"
#define MSG_PROMPT_INPUT_PGP_ENTRY_DIGIT        @"%@は%d～%d文字で入力してください"
#define MSG_PROMPT_INPUT_PGP_ASCII_ENTRY        @"%@は半角文字で入力してください"
#define MSG_PROMPT_INPUT_PGP_ADDRESS_ENTRY      @"%@を正しく入力してください"
#define MSG_PROMPT_INPUT_PGP_PIN_DIGIT          @"%@は%d桁で入力してください"
#define MSG_PROMPT_INPUT_PGP_ADMIN_PIN_DIGIT    @"%@を8桁で入力してください"
#define MSG_PROMPT_INPUT_PGP_ADMIN_PIN_NUM      @"%@を数字で入力してください"
#define MSG_PROMPT_INPUT_PGP_ADMIN_PIN_CONFIRM  @"%@を正しく入力してください"
#define MSG_PROMPT_INPUT_PGP_ENTRY_NOSP_BOTHEND @"%@の先頭または末尾の半角スペースを除去してください"
#define MSG_PROMPT_INSTALL_PGP_KEY              @"インストールを実行しますか？"
#define MSG_PROMPT_OPENPGP_RESET                @"OpenPGP機能の設定（鍵・PIN番号等）が全て削除され、OpenPGP機能が使用できなくなります。\n\n処理を開始しますか？"
#define MSG_PROMPT_OPENPGP_PIN_COMMAND          @"処理を実行しますか？"
#define MSG_LABEL_PGP_REAL_NAME                 @"名前"
#define MSG_LABEL_PGP_MAIL_ADDRESS              @"メールアドレス"
#define MSG_LABEL_PGP_COMMENT                   @"コメント"
#define MSG_LABEL_PGP_ADMIN_PIN                 @"OpenPGP機能の管理用PIN"
#define MSG_LABEL_PGP_ADMIN_PIN_CONFIRM         @"OpenPGP機能の管理用PIN（確認）"
#define MSG_LABEL_ITEM_PGP_PIN                  @"PIN番号"
#define MSG_LABEL_ITEM_PGP_ADMIN_PIN            @"管理用PIN番号"
#define MSG_LABEL_ITEM_PGP_RESET_CODE           @"リセットコード"
#define MSG_LABEL_ITEM_CUR_PIN                  @"現在のPIN番号"
#define MSG_LABEL_ITEM_NEW_PIN                  @"新しいPIN番号"
#define MSG_LABEL_ITEM_CUR_ADMPIN               @"現在の管理用PIN番号"
#define MSG_LABEL_ITEM_NEW_ADMPIN               @"新しい管理用PIN番号"
#define MSG_LABEL_ITEM_CUR_RESET_CODE           @"現在のリセットコード"
#define MSG_LABEL_ITEM_NEW_RESET_CODE           @"新しいリセットコード"
#define MSG_LABEL_COMMAND_OPENPGP_INSTALL_KEYS  @"PGP秘密鍵のインストール"
#define MSG_LABEL_COMMAND_OPENPGP_STATUS        @"設定情報の参照"
#define MSG_LABEL_COMMAND_OPENPGP_RESET         @"設定情報の消去"
#define MSG_LABEL_COMMAND_OPENPGP_CHANGE_PIN        @"PIN番号変更"
#define MSG_LABEL_COMMAND_OPENPGP_CHANGE_ADMIN_PIN  @"管理用PIN番号変更"
#define MSG_LABEL_COMMAND_OPENPGP_UNBLOCK_PIN       @"PIN番号リセット"
#define MSG_LABEL_COMMAND_OPENPGP_SET_RESET_CODE    @"リセットコード変更"
#define MSG_LABEL_COMMAND_OPENPGP_UNBLOCK           @"リセットコードによるPIN番号リセット"
#define MSG_LABEL_COMMAND_OPENPGP_ADMIN_PIN_VERIFY  @"管理用PIN番号の検証"

#pragma mark - OATH機能関連
#define MSG_ERROR_OATH_SCREENSHOT_PERMISSION        @"画面収録が許可されていません。"
#define MSG_INFORMATIVE_OATH_SCREENSHOT_PERMISSION  @"システム環境設定の「セキュリティとプライバシー」で「%@」に画面収録を許可するよう設定してください。"
#define MSG_ERROR_OATH_QRCODE_SCAN_FAILED           @"認証用QRコードが画面からスキャンできませんでした。"
#define MSG_ERROR_OATH_SCANNED_ACCOUNT_INFO_INVALID @"認証用QRコードからスキャンしたアカウント情報が不正です。"
#define MSG_ERROR_OATH_APPLET_SELECT_FAILED         @"OATH機能を使用することができません。"
#define MSG_ERROR_OATH_ACCOUNT_ADD_APDU_FAILED      @"OATHアカウント登録処理用のリクエストデータ生成に失敗しました。"
#define MSG_ERROR_OATH_ACCOUNT_ADD_FAILED           @"認証器にOATHアカウント登録時、エラーが発生しました（SW=0x%04x）"
#define MSG_INFO_OATH_ACCOUNT_ADD_SUCCESS           @"認証器にOATHアカウントを登録しました。"
#define MSG_ERROR_OATH_CALCULATE_APDU_FAILED        @"OATHワンタイムパスワード生成処理用のリクエストデータ生成に失敗しました。"
#define MSG_ERROR_OATH_CALCULATE_FAILED             @"OATHワンタイムパスワード生成時、エラーが発生しました（SW=0x%04x）"
#define MSG_INFO_OATH_CALCULATE_SUCCESS             @"OATHワンタイムパスワードを生成しました。"
#define MSG_ERROR_OATH_LIST_ACCOUNT_FAILED          @"OATHアカウント一覧の取得時、エラーが発生しました（SW=0x%04x）"
#define MSG_ERROR_OATH_LIST_ACCOUNT_DATA_INVALID    @"認証器から取得したOATHアカウント一覧のデータが不正です。"
#define MSG_ERROR_OATH_ACCOUNT_DELETE_APDU_FAILED   @"OATHアカウント削除処理用のリクエストデータ生成に失敗しました。"
#define MSG_ERROR_OATH_ACCOUNT_DELETE_FAILED        @"OATHアカウント削除時、エラーが発生しました（SW=0x%04x）"
#define MSG_LABEL_COMMAND_OATH_GENERATE_TOTP        @"ワンタイムパスワードの生成"
#define MSG_LABEL_COMMAND_OATH_UPDATE_TOTP          @"ワンタイムパスワードの更新"
#define MSG_LABEL_COMMAND_OATH_LIST_ACCOUNT         @"OATHアカウント一覧の取得"
#define MSG_LABEL_COMMAND_OATH_DELETE_ACCOUNT       @"OATHアカウントの削除"
#define MSG_TITLE_OATH_DELETE_ACCOUNT               @"OATHアカウントを削除します。"
#define MSG_PROMPT_OATH_DELETE_ACCOUNT              @"削除後は、%@のワンタイムパスワードが参照できなくなります。\n削除処理を開始しますか？"
#define MSG_TITLE_OATH_ACCOUNT_SEL_FOR_TOTP         @"ワンタイムパスワードを参照するアカウントの選択"
#define MSG_CAPTION_OATH_ACCOUNT_SEL_FOR_TOTP       @"ワンタイムパスワードを参照したいアカウントを、\n下のリストから選択して下さい。"
#define MSG_TITLE_OATH_ACCOUNT_SEL_FOR_DELETE       @"削除するアカウントの選択"
#define MSG_CAPTION_OATH_ACCOUNT_SEL_FOR_DELETE     @"認証器から削除したいアカウントを、\n下のリストから選択して下さい。"

#pragma mark - ベンダー向け機能関連
#define MSG_INVALID_SKEY_LENGTH_IN_PEM              @"鍵ファイルに格納された秘密鍵の長さが不正です。"
#define MSG_INVALID_SKEY_HEADER_IN_PEM              @"鍵ファイルに格納された秘密鍵のヘッダーが不正です。"
#define MSG_CANNOT_READ_SKEY_PEM_FILE               @"鍵ファイルを読み込むことができません。"
#define MSG_INVALID_SKEY_CONTENT_IN_PEM             @"鍵ファイルの内容が不正です。"
#define MSG_CANNOT_READ_CERT_CRT_FILE               @"証明書ファイルを読み込むことができません。"
#define MSG_INVALID_CERT_LENGTH_IN_CRT              @"証明書ファイルに格納されたデータの長さが不正です。"
#define MSG_INVALID_SKEY_OR_CERT                    @"秘密鍵または公開鍵の内容が不正です。"
#define MSG_CHANGE_TO_BOOTLOADER_MODE               @"FIDO認証器をブートローダーモードに遷移させます。"
#define MSG_PROMPT_CHANGE_TO_BOOTLOADER_MODE        @"ブートローダーモードに遷移したら、nRFコマンドラインツール等を使用し、ファームウェア更新イメージをFIDO認証器に転送できます。\n遷移処理を実行しますか？"
#define MSG_FIRMWARE_RESET                          @"FIDO認証器のファームウェアを再起動します。"
#define MSG_PROMPT_FIRMWARE_RESET                   @"処理を実行しますか？"

#endif /* AppCommonMessage_h */
