//
//  ToolCommonMessage.h
//  U2FMaintenanceTool
//
//  Created by Makoto Morita on 2018/02/26.
//
#ifndef ToolCommonMessage_h
#define ToolCommonMessage_h

#pragma mark - 共通
#define MSG_INVALID_FIELD           @"入力値が不正です。"
#define MSG_INVALID_FILE_PATH       @"ファイルが存在しません。"
#define MSG_INVALID_NUMBER          @"入力値が数値ではありません。"
#define MSG_BUTTON_SELECT           @"選択"
#define MSG_BUTTON_CREATE           @"作成"
#define MSG_SUCCESS                 @"成功"
#define MSG_FAILURE                 @"失敗"

#pragma mark - ホーム画面
#define MSG_PROMPT_SELECT_PKEY_PATH @"秘密鍵ファイル(PEM)のパスを選択してください"
#define MSG_PROMPT_SELECT_CRT_PATH  @"証明書ファイル(CRT)のパスを選択してください"
#define MSG_SETUP_CHROME            @"ChromeでBLE U2Fトークンが使用できるよう設定します。"
#define MSG_PROMPT_SETUP_CHROME     @"ChromeでBLE U2Fトークンを使用時、このU2F管理ツールがChromeのサブプロセスとして起動します。\n設定を実行しますか？"
#define MSG_ERASE_SKEY_CERT         @"One Cardから鍵・証明書・キーハンドルをすべて削除します。"
#define MSG_PROMPT_ERASE_SKEY_CERT  @"削除後はOne CardによるU2F認証ができなくなります。\n削除処理を実行しますか？"
#define MSG_OCCUR_BLECONN_ERROR     @"BLE接続エラーが発生しました。"
#define MSG_OCCUR_KEYHANDLE_ERROR   @"キーハンドルが存在しません。再度U2F Register(Enroll)を実行してください。"
#define MSG_OCCUR_FDS_GC_ERROR      @"One CardのFlash ROM領域が一杯になり処理が中断されました(領域は自動再編成されます)。\n処理を再試行してください。"
#define MSG_OCCUR_UNKNOWN_BLE_ERROR @"BLEエラーが発生しました。処理を再試行してください。"
#define MSG_OCCUR_SKEYNOEXIST_ERROR @"鍵・証明書がインストールされていません。鍵・証明書インストール処理を実行してください。"
#define MSG_OCCUR_UNKNOWN_ERROR     @"不明なエラーが発生しました。"
#define MSG_FORMAT_END_MESSAGE      @"%1$@が%2$@しました。"

#pragma mark - 証明書要求ファイル作成画面
#define MSG_PROMPT_SELECT_PEM_PATH  @"使用する秘密鍵ファイル(PEM)を選択してください。"
#define MSG_PROMPT_CREATE_CSR_PATH  @"作成する証明書要求ファイル(CSR)名を指定してください。"
#define MSG_PROMPT_CREATE_PEM_PATH  @"作成する秘密鍵ファイル(PEM)名を指定してください。"
#define MSG_PROMPT_INPUT_CN         @"実際に接続されるURLのFQDN（例：www.diverta.co.jp）を入力してください。"
#define MSG_PROMPT_INPUT_O          @"申請組織の名称（例：Diverta Inc.）を入力してください。"
#define MSG_PROMPT_INPUT_L          @"申請組織の事業所住所の市区町村名（例：Shinjuku-ku、Yokohama-shi等）を入力してください。"
#define MSG_PROMPT_INPUT_ST         @"申請組織の事業所住所の都道府県名（例：Tokyo、Kanagawa）を入力してください。"
#define MSG_PROMPT_INPUT_C          @"申請組織の事業所住所の国名（例：JP）を入力してください。"
#define MSG_PROMPT_EXIST_PEM_PATH   @"実在する秘密鍵ファイル(PEM)のパスを指定してください。"

#pragma mark - 自己署名証明書ファイル作成画面
#define MSG_PROMPT_SELECT_CSR_PATH  @"使用する証明書要求ファイル(CSR)を選択してください。"
#define MSG_PROMPT_CREATE_CRT_PATH  @"作成する自己署名証明書ファイル(CRT)名を指定してください。"
#define MSG_PROMPT_EXIST_CSR_PATH   @"実在する証明書要求ファイル(CSR)のパスを指定してください。"
#define MSG_PROMPT_INPUT_CRT_DAYS   @"自己署名証明書の有効期間（日数）を数値で入力してください。"

#pragma mark - コマンド種別に対応する処理名称
#define PROCESS_NAME_ERASE_BOND                     @"ペアリング情報削除処理"
#define PROCESS_NAME_ERASE_SKEY_CERT                @"鍵・証明書・キーハンドル削除処理"
#define PROCESS_NAME_INSTALL_SKEY_CERT              @"鍵・証明書インストール"
#define PROCESS_NAME_HEALTHCHECK                    @"ヘルスチェック"
#define PROCESS_NAME_U2F_PROCESS                    @"Chromeブラウザから要求された処理"
#define PROCESS_NAME_SETUP_CHROME_NATIVE_MESSAGING  @"Chrome Native Messaging有効化設定"
#define PROCESS_NAME_CREATE_KEYPAIR_PEM             @"鍵ファイル作成"
#define PROCESS_NAME_CREATE_CERTREQ_CSR             @"証明書要求ファイル作成"
#define PROCESS_NAME_CREATE_SELFCRT_CRT             @"自己署名証明書ファイル作成"
#define PROCESS_NAME_PAIRING                        @"ペアリング"

#pragma mark - ToolCommandクラス専用メッセージ
#define MSG_INVALID_SKEY_LENGTH_IN_PEM      @"鍵ファイルに格納された秘密鍵の長さが不正です。"
#define MSG_INVALID_SKEY_HEADER_IN_PEM      @"鍵ファイルに格納された秘密鍵のヘッダーが不正です。"
#define MSG_CANNOT_READ_SKEY_PEM_FILE       @"鍵ファイルを読み込むことができません。"
#define MSG_INVALID_SKEY_CONTENT_IN_PEM     @"鍵ファイルの内容が不正です。"
#define MSG_CANNOT_READ_CERT_CRT_FILE       @"証明書ファイルを読み込むことができません。"
#define MSG_INVALID_CERT_LENGTH_IN_CRT      @"証明書ファイルに格納されたデータの長さが不正です。"
#define MSG_READ_NBYTES_FROM_CRT_FILE       @"証明書ファイル(%ldバイト)を読込みました。%@"
#define MSG_OCCUR_JSON_STRING_ERROR         @"設定用JSON文字列生成に失敗しました。処理を再試行してください。"
#define MSG_OCCUR_JSON_DIRGET_ERROR         @"設定用JSONファイルインストール先ディレクトリー取得に失敗しました。処理を再試行してください。"
#define MSG_OCCUR_JSON_OUTPUT_ERROR         @"設定用JSONファイルの出力に失敗しました。処理を再試行してください。"
#define MSG_RESPONSE_RECEIVED               @"レスポンスを受信しました。"

#pragma mark - ToolBLECentralクラス専用メッセージ
#define MSG_INVALID_BLE_PERIPHERAL          @"BLEが無効化されています。BLEを有効にしてください。"
#define MSG_U2F_DEVICE_SCAN_START           @"FIDO U2Fデバイスのスキャンを開始します。"
#define MSG_U2F_DEVICE_SCAN_STOPPED         @"FIDO U2Fデバイスのスキャンを停止しました。"
#define MSG_U2F_DEVICE_SCAN_TIMEOUT         @"FIDO U2Fデバイスのスキャンがタイムアウトしました。"
#define MSG_U2F_DEVICE_CONNREQ_TIMEOUT      @"FIDO U2Fデバイスの接続要求がタイムアウトしました。"
#define MSG_U2F_DEVICE_CONNECTED            @"FIDO U2Fデバイスに接続しました。"
#define MSG_U2F_DEVICE_CONNECT_FAILED       @"FIDO U2Fデバイスの接続に失敗しました。"
#define MSG_U2F_DEVICE_DISCONNECTED         @"FIDO U2Fデバイスの接続を切断しました。"
#define MSG_BLE_SERVICE_NOT_DISCOVERED      @"BLEサービスが見つかりません。"
#define MSG_BLE_U2F_SERVICE_FOUND           @"FIDO BLE U2Fサービスが見つかりました。"
#define MSG_BLE_U2F_SERVICE_NOT_FOUND       @"FIDO BLE U2Fサービスが見つかりません。"
#define MSG_BLE_CHARACT_NOT_DISCOVERED      @"FIDO BLE U2Fサービスと通信できません。"
#define MSG_BLE_CHARACT_NOT_EXIST           @"FIDO BLE U2Fサービスからデータを受信できる項目が存在しません。"
#define MSG_BLE_NOTIFICATION_FAILED         @"FIDO BLE U2Fサービスからデータを受信できません。"
#define MSG_BLE_NOTIFICATION_START          @"受信データの監視を開始します。"
#define MSG_BLE_NOTIFICATION_STOP           @"受信データの監視を停止します。"
#define MSG_REQUEST_SENT                    @"リクエストを送信しました。"
#define MSG_REQUEST_TIMEOUT                 @"リクエストがタイムアウトしました。"
#define MSG_REQUEST_SEND_FAILED             @"リクエスト送信が失敗しました。"
#define MSG_RESPONSE_RECEIVE_FAILED         @"レスポンスを受信できませんでした。"
#define MSG_DISCOVER_U2F_SERVICES_TIMEOUT   @"FIDO BLE U2Fサービスの検索がタイムアウトしました。"
#define MSG_DISCOVER_U2F_CHARAS_TIMEOUT     @"FIDO BLE U2Fサービス送受信項目の検索がタイムアウトしました。"
#define MSG_SUBSCRIBE_U2F_STATUS_TIMEOUT    @"FIDO BLE U2Fサービス受信項目の監視ステータス更新がタイムアウトしました。"

#pragma mark - 接続再試行関連メッセージ
#define MSG_BLE_CONNECTION_RETRY_WITH_CNT   @"処理中にBLE接続が消失しました。接続を再試行しています（%lu回目）"
#define MSG_BLE_CONNECTION_RETRY_END        @"処理中にBLE接続が消失しましたが、接続再試行の上限回数に達したため終了します。"

#pragma mark - ペアリング情報消去関連メッセージ
#define MSG_ERASE_BONDING_INFO              @"One Cardからペアリング情報をすべて消去します。"
#define MSG_PROMPT_ERASE_BONDING_INFO       @"消去後は、すべてのPCでOne Cardとの通信ができなくなります。\n消去処理を実行しますか？"

#endif /* ToolCommonMessage_h */
